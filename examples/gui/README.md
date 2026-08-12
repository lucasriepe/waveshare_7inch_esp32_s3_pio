# Examples

Drop-in snippets that show how to use the `waveshare_driver` library
together with LVGL and the on-board peripherals. Each file is guarded by
`#pragma once` style include guards and can be `#include`-d directly into
`src/main.cpp`.

| File                                   | Demonstrates                                         |
| -------------------------------------- | ---------------------------------------------------- |
| [`hello_label.cpp`](hello_label.cpp)   | Centred LVGL label updated by a free-running counter |
| [`touch_button.cpp`](touch_button.cpp) | LVGL button responding to GT911 touch events         |
| [`sd_logger.cpp`](sd_logger.cpp)       | Periodic ADC voltage logging to `/sdcard/log.csv`    |
| [`can_echo.cpp`](can_echo.cpp)         | TWAI echo loop (RX → TX) at 500 kbps                 |

## Adding examples to the sketch

Pick the snippets you want and include them from `src/main.cpp`. The
helpers are exposed both inside the `waveshare_examples` namespace and at
global scope, so you can call them directly:

```cpp
#include <Arduino.h>
#include <waveshare_driver/waveshare_driver.hpp>

#include "examples/gui/hello_label.cpp"
#include "examples/gui/touch_button.cpp"
#include "examples/gui/sd_logger.cpp"
#include "examples/gui/can_echo.cpp"

using namespace waveshare;

void setup() {
    Serial.begin(115200);

    // LCD + LVGL + GT911 touch + backlight
    LcdPanel::begin(/*enable_backlight=*/true);

    // External peripherals
    AdcSensor::begin();
    SdCard::begin(/*format_if_mount_failed=*/false);

    // Build widgets and start background services
    touch_button_create();
    hello_label_create();
    sd_logger_begin();
    can_echo_begin(/*bitrate=*/500000);
}

void loop() {
    LcdPanel::tick(5);    // LVGL handler + yield
    hello_label_tick();   // label refresh (~500 ms)
    sd_logger_tick();     // ADC log append (~1 s)
    can_echo_tick();      // drain CAN RX queue
}
```

## Conventions

- **Static state.** Every example keeps its module state in
  `static` variables inside `namespace waveshare_examples` so two
  examples can coexist in the same sketch without symbol clashes.
- **Idempotent constructors.** `*_create()` and `*_begin()` functions
  short-circuit if the underlying widget / handle already exists.
- **Non-blocking ticks.** `*_tick()` helpers return immediately when the
  configured period has not elapsed. Call them as often as you like —
  typically once per `loop()`.

## Selecting individual examples

You do not have to include all four snippets. A minimal sketch that
only drives the label and touch button looks like this:

```cpp
#include <Arduino.h>
#include <waveshare_driver/waveshare_driver.hpp>

#include "examples/gui/hello_label.cpp"
#include "examples/gui/touch_button.cpp"

using namespace waveshare;

void setup() {
    LcdPanel::begin(true);
    touch_button_create();
}

void loop() {
    LcdPanel::tick(5);
    hello_label_tick();
}
```

> Note: `examples/gui/*.cpp` files are excluded from the default build
> by the `src_filter = +<main.cpp>` rule in `platformio.ini`. They are
> pulled in explicitly through `#include "examples/gui/..."` so the
> linker can resolve the symbols they define.
