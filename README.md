# Waveshare ESP32-S3 Touch LCD 7 — Driver Library

Arduino-style C++ wrappers for the **Waveshare ESP32-S3-Touch-LCD-7** development
board (800×480 RGB LCD, GT911 touch, CH422G I/O expander, SD card, TWAI/CAN,
ADC, UART).

The library is **header-only**, lives under
[`include/waveshare_driver/`](../../), and is generated from the
per-peripheral reference projects in the `boardport/` directory.

```cpp
#include <Arduino.h>
#include <waveshare_driver/waveshare_driver.hpp>

using namespace waveshare;
```

---

## Table of contents

| Class                                                                | Purpose                                               |
| -------------------------------------------------------------------- | ----------------------------------------------------- |
| [`BoardConfig`](#boardconfig--pins--constants)                       | Pin & constant tables (single source of truth)        |
| [`I2CBus`](#i2cbus--shared-i2c-bus)                                  | Shared I²C master (SDA=8, SCL=9, 400 kHz)             |
| [`ExpanderCH422G`](#expanderch422g--backlight--touch-reset--usb-mux) | CH422G I/O expander (backlight, USB mux, touch reset) |
| [`TouchGT911`](#touchgt911--capacitive-touch)                        | GT911 polling (I²C 0x5D / 0x14)                       |
| [`LcdPanel`](#lcdpanel--rgb-lcd--lvgl-wiring)                        | RGB-LCD + LVGL + Touch wiring                         |
| [`SdCard`](#sdcard--sd-card-over-spi)                                | SD card over SPI through CH422G                       |
| [`TwaiBus`](#twaibus--can-bus)                                       | TWAI (CAN) transmit / receive                         |
| [`AdcSensor`](#adcsensor--adc1_ch5-sensor)                           | ADC1_CH5 (GPIO6) analogue input                       |
| [`UartEcho`](#uartecho--uart2-echo)                                  | UART2 echo task                                       |

---

## Module overview

### `BoardConfig` — pins & constants

All GPIO numbers, signal frequencies, I²C addresses and timing constants are
exposed as `constexpr` values in `namespace waveshare`. The values are taken
verbatim from the original `boardport/` reference projects.

```cpp
#include <waveshare_driver/BoardConfig.hpp>

using namespace waveshare;

constexpr int sda  = PIN_I2C_SDA;   // 8
constexpr int scl  = PIN_I2C_SCL;   // 9
constexpr int tx   = PIN_TWAI_TX;   // 0
constexpr int rx   = PIN_TWAI_RX;   // 2
constexpr int mosi = PIN_SD_MOSI;   // 35
constexpr int cs   = PIN_SD_CS;     // 34
```

Notable constants:

| Name                                    | Value           | Meaning                       |
| --------------------------------------- | --------------- | ----------------------------- |
| `LCD_WIDTH`, `LCD_HEIGHT`               | `800`, `480`    | display geometry              |
| `LCD_PIXEL_CLOCK_HZ`                    | `16 000 000`    | RGB pixel clock               |
| `I2C_CLOCK_HZ`                          | `400 000`       | shared I²C bus speed          |
| `I2C_EXPANDER_CFG_ADDR`                 | `0x24`          | CH422G configuration register |
| `I2C_EXPANDER_DATA_ADDR`                | `0x38`          | CH422G data register          |
| `CH422G_BL_ON` / `_OFF`                 | `0x1E` / `0x1A` | backlight state bytes         |
| `CH422G_USB_SEL_HIGH`                   | `0x20`          | routes GPIO0/2 to CAN         |
| `TOUCH_I2C_ADDR_PRIMARY` / `_SECONDARY` | `0x5D` / `0x14` | GT911 addresses               |
| `SD_SPI_CLOCK_HZ`                       | `20 000 000`    | SD card SPI clock             |

---

### `I2CBus` — shared I²C bus

The RGB panel, GT911 touch, CH422G expander, SD card and TWAI USB-mux all share
**one** I²C controller (`I2C_NUM_0`) on **SDA=8 / SCL=9** at **400 kHz**.

```cpp
#include <waveshare_driver/I2CBus.hpp>

using namespace waveshare;

// Bring the shared bus up (idempotent)
I2CBus::begin(Wire);             // uses PIN_I2C_SDA / PIN_I2C_SCL / 400 kHz by default
I2CBus::begin(Wire, /*sda=*/8, /*scl=*/9, /*clock_hz=*/400000);

// Low-level helpers
bool ok = I2CBus::writeByte(0x38, 0x1E);                              // backlight on
size_t n = I2CBus::readBytes(0x5D, 0x8140, buffer, /*len=*/8);         // read touch ID

TwoWire &bus = I2CBus::wire();
```

`I2CBus::begin()` is **idempotent** — calling it more than once is safe.

---

### `ExpanderCH422G` — backlight / touch-reset / USB mux

The board uses a **CH422G** I/O expander to drive signals that have no direct
GPIO (backlight enable, touch reset, USB → CAN multiplexer).

```cpp
#include <waveshare_driver/ExpanderCH422G.hpp>
using namespace waveshare;

ExpanderCH422G::begin();                          // put CH422G in output mode
ExpanderCH422G::setBacklight(true);               // write CH422G_BL_ON (0x1E)
ExpanderCH422G::setBacklight(false);              // write CH422G_BL_OFF (0x1A)
ExpanderCH422G::pulseTouchReset(/*delay_ms=*/10); // GT911 reset sequence
ExpanderCH422G::selectUsbHigh();                  // route GPIO0/2 → CAN
ExpanderCH422G::selectUsbLow();                   // restore USB
```

Each helper performs the exact byte sequence documented in the
`boardport/08_lvgl_Porting/main/waveshare_rgb_lcd_port.c` reference.

---

### `TouchGT911` — capacitive touch

```cpp
#include <waveshare_driver/TouchGT911.hpp>
using namespace waveshare;

TouchGT911::begin();                  // CH422G reset, then probe 0x5D / 0x14
bool ok = TouchGT911::isReady();

TouchPoint tp;
if (TouchGT911::read(tp)) {
    if (tp.pressed) {
        Serial.printf("x=%d y=%d\n", tp.x, tp.y);
    }
}
```

The driver polls the GT911 status register (`0x814E`), reads the first touch
coordinate pair (`0x814F…0x8154`) and clears the status flag.

---

### `LcdPanel` — RGB-LCD + LVGL wiring

`LcdPanel::begin()` performs the full startup sequence:

1. Initialises the shared I²C bus.
2. Puts the CH422G expander into output mode.
3. Pulses the GT911 touch reset.
4. Brings up LVGL v9 with a partial render buffer (100 lines).
5. Registers the GT911 input device with LVGL.
6. (Optional) Switches the backlight on.

```cpp
#include <waveshare_driver/LcdPanel.hpp>
using namespace waveshare;

void setup() {
    Serial.begin(115200);
    bool ok = LcdPanel::begin(/*enable_backlight=*/true);
    if (!ok) {
        Serial.println("LCD bring-up failed");
        ESP.restart();
    }
}

void loop() {
    LcdPanel::tick(5);                 // lv_task_handler() + small delay
}
```

Other helpers:

```cpp
LcdPanel::setBacklight(false);         // turn panel off
lv_display_t *disp = LcdPanel::display();
```

> Note: the flush callback in `LcdPanel` is a stub. Production builds must
> dispatch to `esp_lcd_panel_draw_bitmap()` from the flush callback and call
> `lv_display_flush_ready()` from the panel's vsync callback.

---

### `SdCard` — SD card over SPI

```cpp
#include <waveshare_driver/SdCard.hpp>
using namespace waveshare;

void setup() {
    SdCard::begin(/*format_if_mount_failed=*/false);
    if (SdCard::isMounted()) {
        Serial.println("SD card mounted at /sdcard");
    }
}

// File helpers
SdCard::writeFile("/sdcard/log.txt", "hello\n");
String content = SdCard::readFile("/sdcard/log.txt");
bool exists = SdCard::exists("/sdcard/log.txt");

// Self-test (mirrors waveshare_sd_card_test from boardport)
SdCard::runSelfTest(/*format_after=*/false);

// Tear-down
SdCard::end();
```

Pin assignment: `MOSI=35`, `MISO=37`, `CLK=36`, `CS=34`.

---

### `TwaiBus` — CAN bus

The board's CAN TX/RX lines (GPIO0/GPIO2) are shared with USB through the
FSUSB42UMX mux. `TwaiBus::begin()` selects **CAN** mode automatically.

```cpp
#include <waveshare_driver/TwaiBus.hpp>
using namespace waveshare;

void setup() {
    TwaiBus::begin(/*bitrate=*/500000, TwaiRole::Normal);
}

void loop() {
    uint8_t payload[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0, 0, 0, 0};
    TwaiBus::transmit(/*id=*/0x123, payload, /*len=*/4);

    TwaiMessage rx;
    if (TwaiBus::receive(rx, /*timeout_ms=*/100)) {
        Serial.printf("RX id=0x%03X dlc=%u data[0]=0x%02X\n",
                      rx.identifier, rx.data_length_code, rx.data[0]);
    }
}
```

Modes:

```cpp
TwaiBus::begin(500000, TwaiRole::Normal);     // typical transmit
TwaiBus::begin(500000, TwaiRole::ListenOnly); // monitor, no ACK
TwaiBus::begin(500000, TwaiRole::NoAck);      // transmit, ignore ACK
```

Built-in helpers map the `boardport/06_TWAItransmit` reference samples:

```cpp
TwaiBus::transmitTestFrame();                  // id=0x0F6, data={0,1,…,7}
TwaiBus::runTransmitterLoop();                 // transmit every 1000 ms
```

---

### `AdcSensor` — ADC1_CH5 sensor

```cpp
#include <waveshare_driver/AdcSensor.hpp>
using namespace waveshare;

AdcSensor::begin();

int raw_mv = AdcSensor::readMillivolts();      // calibrated mV (eFuse if available)
int raw    = AdcSensor::readRaw();             // 0..4095 raw count

// Cooperative sampling (boardport loop pattern)
AdcSensor::runSamplingLoop(/*period_ms=*/1000, [](int mv) {
    Serial.printf("Sensor: %d mV\n", mv);
    return true;
});
```

The driver uses `GPIO6` which is `ADC1_CH5` on the ESP32-S3.

---

### `UartEcho` — UART2 echo

```cpp
#include <waveshare_driver/UartEcho.hpp>
using namespace waveshare;

void setup() {
    UartEcho::begin(Serial2, /*baud=*/115200, /*tx=*/4, /*rx=*/5);
}

// Write directly via the Serial handle
UartEcho::serial().println("Hello UART2");
```

A dedicated `xTaskCreate`-based echo task is started by `UartEcho::begin()`.

---

## Design rules

1. **One I²C bus.** Every class that needs I²C goes through `I2CBus`. It is
   brought up by the first peripheral that requires it and never reinitialised.
2. **Header-only.** No `.cpp` files, no `lib_deps` changes.
3. **`constexpr` constants.** `BoardConfig` lets users reference pins and
   timings directly without consulting runtime objects.
4. **Idempotent `begin()`.** Each module protects its initialiser with an
   internal `s_ready` flag.
5. **Static API.** No global state is exposed besides the small number of
   module-level flags inside each class.

## Testing on hardware

```bash
# Build
pio run

# Upload + open serial monitor
pio run -t upload && pio device monitor
```

Quick smoke-test sketch (`src/main.cpp`):

```cpp
#include <Arduino.h>
#include <waveshare_driver/waveshare_driver.hpp>
using namespace waveshare;

void setup() {
    Serial.begin(115200);
    LcdPanel::begin(true);
    AdcSensor::begin();
    SdCard::begin();
    TwaiBus::begin();
    UartEcho::begin();
}

void loop() {
    LcdPanel::tick(5);
    AdcSensor::runSamplingLoop();
    TwaiBus::runTransmitterLoop();
}
```

## License

MIT.
