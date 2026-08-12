// =====================================================================
//  main.cpp — Arduino sketch for the Waveshare ESP32-S3-Touch-LCD-7
//  ----------------------------------------------------------------------
//  This file only contains the Arduino entry points plus the includes
//  for the driver library. All application code (peripheral bring-up,
//  LVGL widgets, CAN traffic, ADC sampling, SD logging) is added
//  directly here, or pulled in from examples/gui/*.cpp once the user
//  is ready to extend the project.
//
//  The default sketch brings up the RGB LCD + LVGL + GT911 touch and
//  delegates the main loop to LcdPanel::tick(). Extend setup() and
//  loop() below with your own logic, or include example snippets:
//
//      #include "examples/gui/hello_label.cpp"
//
//  See examples/gui/README.md for the full list.
// =====================================================================

#include <Arduino.h>
#include <waveshare_driver/waveshare_driver.hpp>

using namespace waveshare;

void setup() {
    // Initialise the debug console first so subsequent failures can be
    // reported over USB.
    Serial.begin(115200);

    // Bring up the RGB-LCD, LVGL, GT911 touch input and the backlight.
    // TouchGT911::begin() and ExpanderCH422G::begin() are invoked from
    // inside LcdPanel::begin() automatically.
    const bool ok = LcdPanel::begin(/*enable_backlight=*/true);
    if (!ok) {
        Serial.println("[boot] ERROR: LCD / LVGL bring-up failed");
        ESP.restart();
    }

    // Application bring-up code goes here.
}

void loop() {
    // Drives LVGL's task handler and yields for ~5 ms. Replace with
    // additional periodic services as required.
    LcdPanel::tick(5);

    // Application main loop code goes here.
}