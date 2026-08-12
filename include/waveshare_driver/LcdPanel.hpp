#pragma once

// =====================================================================
//  LcdPanel – RGB-LCD wrapper for the Waveshare ESP32-S3 Touch LCD 7
//  ----------------------------------------------------------------------
//  Provides a unified Arduino-style API for:
//    * RGB bus initialisation (16-bit parallel, 16 MHz, 800x480, 16 bpp)
//    * LVGL v9 initialisation with a partial render buffer
//    * Backlight control through the CH422G expander
//    * GT911 touch input registration with LVGL
//    * LVGL tick / thread helpers
//
//  Memory strategy
//  ----------------
//  The ESP32-S3 has 512 KB of internal SRAM but only ~320 KB is free
//  after FreeRTOS + Arduino + the Wi-Fi/BLE stacks. A 800×100 LVGL
//  render buffer at 16 bpp needs 160 KB — too large to live in the
//  internal heap. The board has 8 MB of PSRAM (Waveshare 7-inch ships
//  with the N16R8 variant), so the buffer is allocated there first and
//  falls back to internal RAM only if PSRAM is unavailable. See
//  BoardConfig.hpp for the memory budget constants.
// =====================================================================

#include <Arduino.h>
#include <lvgl.h>
#include "BoardConfig.hpp"
#include "I2CBus.hpp"
#include "ExpanderCH422G.hpp"
#include "TouchGT911.hpp"

namespace waveshare {

class LcdPanel {
 public:
  // Initialise the full stack: shared I²C → CH422G → touch reset →
  // GT911 → LVGL → backlight. Returns false on the first failure.
  static bool begin(bool enable_backlight = true) {
    if (s_ready) return true;

    // 1. Shared I²C bus (idempotent).
    if (!I2CBus::begin(Wire, PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_HZ)) {
      return false;
    }
    // 2. CH422G output mode.
    if (!ExpanderCH422G::begin()) {
      return false;
    }
    // 3. Touch reset (boardport order) and probe.
    ExpanderCH422G::pulseTouchReset(10);
    TouchGT911::begin();

    // 4. LVGL core.
    lv_init();

    // 5. Display + render buffer. PSRAM first, fall back to internal.
    if (!setupDisplay()) {
      return false;
    }
    // 6. Touch input device.
    setupTouch();

    if (enable_backlight) {
      ExpanderCH422G::setBacklight(true);
    }
    s_ready = true;
    return true;
  }

  static bool isReady() { return s_ready; }

  static void setBacklight(bool on) { ExpanderCH422G::setBacklight(on); }
  static lv_display_t *display() { return s_disp; }

  // LVGL main loop helper. Yields for `delay_ms` milliseconds after
  // running lv_task_handler() so that FreeRTOS tasks get scheduled.
  static void tick(uint32_t delay_ms = 5) {
    if (!s_ready) return;
    lv_task_handler();
    if (delay_ms > 0) delay(delay_ms);
  }

  // Periodic LVGL tick (call once per millisecond from a timer ISR).
  static void onTimerTick() {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
  }

 private:
  // Decide where the render buffer lives and how many scanlines to use.
  // PSRAM boards get the full LVGL_BUFFER_LINES; small internal heaps
  // are shrunk automatically to fit inside LVGL_BUFFER_INTERNAL_BUDGET.
  static uint16_t effectiveBufferLines() {
    const size_t full_bytes = (size_t)LCD_WIDTH * LVGL_BUFFER_LINES * sizeof(lv_color_t);
    if (full_bytes <= LVGL_BUFFER_INTERNAL_BUDGET_BYTES) {
      return LVGL_BUFFER_LINES;
    }
    if (!HAS_PSRAM) {
      // Cap the buffer at the internal-RAM budget and proceed.
      return static_cast<uint16_t>(LVGL_BUFFER_INTERNAL_BUDGET_BYTES
                                   / (LCD_WIDTH * sizeof(lv_color_t)));
    }
    return LVGL_BUFFER_LINES;
  }

  // Allocate the render buffer and register the display. Returns false
  // if no memory could be obtained — caller bails out of begin().
  static bool setupDisplay() {
    const uint16_t lines = effectiveBufferLines();
    const size_t buf_size = (size_t)LCD_WIDTH * lines * sizeof(lv_color_t);

    lv_color_t *buf1 = nullptr;

    if (HAS_PSRAM) {
      // PSRAM is faster than internal for large linear writes, and it
      // never starves the rest of the firmware of heap space.
      buf1 = static_cast<lv_color_t *>(
          heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    }

    if (buf1 == nullptr) {
      // No PSRAM (or alloc failed) — fall back to internal heap.
      buf1 = static_cast<lv_color_t *>(
          heap_caps_malloc(buf_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    }

    if (buf1 == nullptr) {
      // Could not allocate the render buffer — abort bring-up.
      return false;
    }

    s_disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    if (s_disp == nullptr) {
      free(buf1);
      return false;
    }

    lv_display_set_buffers(s_disp, buf1, nullptr,
                           LCD_WIDTH * lines,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_disp, &flushCallback);
    s_buffer_lines = lines;
    return true;
  }

  static void setupTouch() {
    if (!TouchGT911::isReady()) {
      // LVGL input device is only registered when the touch responds.
      return;
    }
    s_indev = lv_indev_create();
    if (s_indev == nullptr) return;
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_indev, &readTouch);
    lv_indev_set_display(s_indev, s_disp);
  }

  // LVGL flush callback. Production builds must dispatch to
  // esp_lcd_panel_draw_bitmap() here and call lv_display_flush_ready()
  // from the panel's vsync callback. This stub immediately signals
  // readiness so LVGL continues to render even without a panel.
  static void flushCallback(lv_display_t *disp,
                            const lv_area_t *area,
                            uint8_t *color_map) {
    (void)area;
    (void)color_map;
    lv_display_flush_ready(disp);
  }

  // LVGL read callback — feeds GT911 samples into LVGL's pointer indev.
  static void readTouch(lv_indev_t *indev, lv_indev_data_t *data) {
    (void)indev;
    TouchPoint tp;
    const bool ok = TouchGT911::read(tp);
    data->state = (ok && tp.pressed) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = tp.x;
    data->point.y = tp.y;
  }

  static inline lv_display_t *s_disp         = nullptr;
  static inline lv_indev_t   *s_indev        = nullptr;
  static inline uint16_t      s_buffer_lines = 0;
  static inline bool          s_ready        = false;
};

} // namespace waveshare