// =====================================================================
//  hello_label.cpp — minimal LVGL label demo driven by the driver lib
//  ----------------------------------------------------------------------
//  Creates a centred LVGL label and updates its text from an incrementing
//  counter twice per second. Demonstrates how to put a single LVGL widget
//  on the screen after `LcdPanel::begin()` has been called.
//
//  Integration in src/main.cpp:
//
//      #include "examples/gui/hello_label.cpp"
//
//      void setup() {
//          LcdPanel::begin(/*enable_backlight=*/true);
//          hello_label_create();            // one-shot
//      }
//
//      void loop() {
//          LcdPanel::tick(5);
//          hello_label_tick();              // non-blocking, 500 ms cadence
//      }
//
//  The file is safe to include multiple times in the same translation unit
//  thanks to the include guard below.
// =====================================================================

#ifndef WAVESHARE_EXAMPLE_HELLO_LABEL_HPP
#define WAVESHARE_EXAMPLE_HELLO_LABEL_HPP

#include <Arduino.h>
#include <lvgl.h>
#include <waveshare_driver/waveshare_driver.hpp>

using namespace waveshare;

namespace waveshare_examples {

// Module state. Marked static so multiple includes do not collide.
static lv_obj_t *s_label      = nullptr;
static uint32_t  s_counter    = 0;
static uint32_t  s_last_tick  = 0;
static constexpr uint32_t kTickPeriodMs = 500;

// Build the label once. Idempotent — calling more than once is safe.
static void hello_label_create() {
  if (s_label != nullptr) return;

  s_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(s_label, lv_color_hex(0x1565C0), LV_PART_MAIN);
  lv_obj_set_style_text_font(s_label, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(s_label, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(s_label, "Hello, Waveshare!");
}

// Service the label. Returns immediately if the period has not elapsed.
static void hello_label_tick() {
  hello_label_create();

  const uint32_t now = millis();
  if (now - s_last_tick < kTickPeriodMs) return;
  s_last_tick = now;

  lv_label_set_text_fmt(s_label, "Hello, Waveshare!  #%u",
                        (unsigned)s_counter++);
}

} // namespace waveshare_examples

// Expose the helpers at the global scope so they can be called from
// Arduino's setup() / loop() without `waveshare_examples::`.
using waveshare_examples::hello_label_create;
using waveshare_examples::hello_label_tick;

#endif // WAVESHARE_EXAMPLE_HELLO_LABEL_HPP