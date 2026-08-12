# test

Arduino project exported from **boardport** for the **Waveshare ESP32-S3 Touch LCD 7** development board (800×480, capacitive GT911 touch, RGB interface, CH422G I/O expander).

This workspace bundles a PlatformIO build and a thin hardware abstraction library
(`include/waveshare_driver/`)

---

## Features

| Peripheral                                            | Status | Wrapper                                                         |
| ----------------------------------------------------- | ------ | --------------------------------------------------------------- |
| RGB LCD (16-bit, 800×480, 16 MHz)                     | ✅     | [`LcdPanel`](include/waveshare_driver/LcdPanel.hpp)             |
| LVGL v9 graphics                                      | ✅     | same class                                                      |
| Capacitive touch (GT911, I²C)                         | ✅     | [`TouchGT911`](include/waveshare_driver/TouchGT911.hpp)         |
| CH422G I/O expander (backlight, USB mux, touch reset) | ✅     | [`ExpanderCH422G`](include/waveshare_driver/ExpanderCH422G.hpp) |
| Shared I²C bus (SDA=8, SCL=9, 400 kHz)                | ✅     | [`I2CBus`](include/waveshare_driver/I2CBus.hpp)                 |
| SD card over SPI                                      | ✅     | [`SdCard`](include/waveshare_driver/SdCard.hpp)                 |
| TWAI / CAN (TX=0, RX=2)                               | ✅     | [`TwaiBus`](include/waveshare_driver/TwaiBus.hpp)               |
| ADC1_CH5 (GPIO6) sensor input                         | ✅     | [`AdcSensor`](include/waveshare_driver/AdcSensor.hpp)           |
| UART2 echo (TX=4, RX=5, 115200)                       | ✅     | [`UartEcho`](include/waveshare_driver/UartEcho.hpp)             |
| All pinouts and constants                             | ✅     | [`BoardConfig`](include/waveshare_driver/BoardConfig.hpp)       |

## Hardware

| Item             | Value                                                 |
| ---------------- | ----------------------------------------------------- |
| MCU              | ESP32-S3                                              |
| Display          | Waveshare ESP32-S3-Touch-LCD-7 (RGB, 800×480, 16 bpp) |
| Touch            | GT911 (I²C addresses 0x5D / 0x14)                     |
| I/O expander     | CH422G (0x24 / 0x38)                                  |
| USB / CAN mux    | FSUSB42UMX (routed via CH422G)                        |
| External SPI     | SD card (MOSI=35, MISO=37, CLK=36, CS=34)             |
| CAN              | TWAI (TX=0, RX=2)                                     |
| ADC sensor input | GPIO6 (ADC1_CH5)                                      |
| UART             | UART2 (TX=4, RX=5)                                    |

The pin map matches the original `boardport/` reference examples
(`08_lvgl_Porting`, `03_SD_Test`, `06_TWAItransmit`, `07_TWAIreceive`,
`04_Sensor_AD`, `02_RS485_Test`, `05_UART_Test`).

## Build

1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI).
2. Open this folder in VS Code or `cd` into it from a terminal.
3. Run:

```bash
pio run
```

PlatformIO automatically downloads the Espressif platform, the Arduino framework,
and `lvgl/lvgl@^9.5.0` on the first build.

To upload to the board:

```bash
pio run -t upload
```

To open the serial monitor:

```bash
pio device monitor
```

## Project layout

```
.
├── README.md                        # this file
├── platformio.ini                   # build configuration (PIO Arduino, lvgl 9.5)
├── include/                         # public headers
│   ├── lv_conf.h                    # LVGL v9 configuration
│   └── waveshare_driver/            # ★ Arduino-style C++ driver library ★
│       ├── README.md
│       ├── waveshare_driver.hpp     # umbrella header
│       ├── BoardConfig.hpp          # pin & constant tables (single source of truth)
│       ├── I2CBus.hpp               # shared I²C bus
│       ├── ExpanderCH422G.hpp       # CH422G backlight / USB / touch-reset
│       ├── TouchGT911.hpp           # GT911 polling
│       ├── LcdPanel.hpp             # RGB-LCD + LVGL wiring
│       ├── SdCard.hpp               # SD over SPI
│       ├── TwaiBus.hpp              # TWAI / CAN
│       ├── AdcSensor.hpp            # ADC1_CH5 sensor
│       └── UartEcho.hpp             # UART2 echo
├── src/
│   └── main.cpp                     # Arduino setup() / loop()
├── examples/
│   └── gui/                         # ★ driver-aware LVGL / driver examples ★
│       ├── README.md
│       ├── hello_label.cpp          # LVGL label that updates from a counter
│       ├── touch_button.cpp         # LVGL button driven by GT911 taps
│       ├── sd_logger.cpp            # periodic ADC logging to /sdcard/log.csv
│       └── can_echo.cpp             # TWAI RX → TX echo loop
└── boardport/                       # original reference examples (read-only)
```

## Driver library

The `include/waveshare_driver/` library exposes every peripheral as a header-only,
Arduino-friendly C++ class. All initialisers are idempotent and use the shared I²C
bus internally.

Minimum startup code:

```cpp
#include <Arduino.h>
#include <waveshare_driver/waveshare_driver.hpp>

using namespace waveshare;

void setup() {
    Serial.begin(115200);
    LcdPanel::begin(/*enable_backlight=*/true);   // RGB LCD + LVGL + Touch + Backlight
}

void loop() {
    LcdPanel::tick(5);                            // runs lv_task_handler()
}
```

See [`include/waveshare_driver/README.md`](include/waveshare_driver/README.md)
for the full API documentation and per-peripheral code samples
(LCD, touch, SD, TWAI, ADC, UART, CH422G).

## Notes

- All pin definitions, display geometry and bus constants are consolidated
  in [`BoardConfig.hpp`](include/waveshare_driver/BoardConfig.hpp). The legacy
  `initBoardDisplay()` / `gui.h` / `pins.h` / `hardware_config.h` headers from
  the original boardport export have been removed; applications should use the
  [`LcdPanel::*`](include/waveshare_driver/LcdPanel.hpp) API instead.
- `platformio.ini` enables `LV_CONF_INCLUDE_SIMPLE`, `BOARD_HAS_PSRAM`, and the
  RGB element-order flags required by the ST7262 panel.
- The driver library is header-only; no extra PlatformIO `lib_deps` entries are
  necessary.
- Driver-aware LVGL snippets live in [`examples/gui/`](examples/gui/README.md).
