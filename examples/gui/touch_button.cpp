// =====================================================================
//  touch_button.cpp — LVGL button driven by GT911 touch input
//  ----------------------------------------------------------------------
//  Requires LcdPanel::begin() to have set up the LVGL input device pointing
//  at the GT911 polling driver. Pressing the button increments a counter
//  that is rendered inside the button label and printed to the serial
//  console.
//
//  Integration in src/main.cpp:
//
//      #include "examples/gui/touch_button.cpp"
//
//      void setup() {
//          LcdPanel::begin(true);
//          touch_button_create();
//      }
//
//      void loop() {
//          LcdPanel::tick(5);
//      }
//
//  The label text is updated from the LVGL event callback; no polling is
//  required from the user.
// =====================================================================

#ifndef WAVESHARE_EXAMPLE_TOUCH_BUTTON_HPP
#define WAVESHARE_EXAMPLE_TOUCH_BUTTON_HPP

#include <Arduino.h>
#include <lvgl.h>
#include <waveshare_driver/waveshare_driver.hpp>

using namespace waveshare;

namespace waveshare_examples {

static lv_obj_t *s_button = nullptr;
static uint32_t  s_clicks = 0;

static void on_button_clicked(lv_event_t *event) {
  (void)event;
  s_clicks++;

  // The label is the first (and only) child of the button.
  lv_obj_t *lbl = lv_obj_get_child(s_button, 0);
  lv_label_set_text_fmt(lbl, "Pressed #%u", (unsigned)s_clicks);

  Serial.printf("[touch_button] clicks=%u\n", (unsigned)s_clicks);
}

// Build the button once. Idempotent.
static void touch_button_create() {
  if (s_button != nullptr) return;

  s_button = lv_button_create(lv_scr_act());
  lv_obj_set_size(s_button, 200, 64);
  lv_obj_align(s_button, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(s_button, on_button_clicked, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *lbl = lv_label_create(s_button);
  lv_label_set_text(lbl, "Tap me");
  lv_obj_center(lbl);
}

// Query the click counter (useful for other modules / tests).
static inline uint32_t touch_button_clicks() { return s_clicks; }

} // namespace waveshare_examples

using waveshare_examples::touch_button_create;
using waveshare_examples::touch_button_clicks;

#endif // WAVESHARE_EXAMPLE_TOUCH_BUTTON_HPP