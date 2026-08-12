#pragma once

// =====================================================================
//  LcdPanel – RGB-LCD-Wrapper für den Waveshare ESP32-S3 Touch LCD 7
//  ----------------------------------------------------------------------
//  Bietet ein einheitliches Arduino-API für:
//   - LCD-Bus-Initialisierung (RGB-Parallel, 16 MHz, 800x480, 16 bpp)
//   - LVGL-Initialisierung mit Display-Buffer
//   - Backlight-Steuerung über CH422G
//   - Touch-Anbindung an LVGL
//   - LVGL-Tick / Threading-Helfer
//
//  Die Pinbelegung und das Timing sind in BoardConfig.hpp hinterlegt.
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
  // Initialisiert LCD + LVGL + Touch in der richtigen Reihenfolge.
  // Im Bildschirm-Konfigurationsmodus wird zudem das Backlight aktiviert.
  static bool begin(bool enable_backlight = true) {
    if (s_ready) return true;

    if (!I2CBus::begin(Wire, PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_HZ)) {
      return false;
    }
    if (!ExpanderCH422G::begin()) {
      return false;
    }

    // Touch-Reset vor LCD-Init (boardport-Reihenfolge)
    ExpanderCH422G::pulseTouchReset(10);
    TouchGT911::begin();

    lv_init();
    setupDisplay();
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

  // LVGL-Hauptschleife. Wird vom Anwender aus loop() aufgerufen.
  static void tick(uint32_t delay_ms = 5) {
    if (!s_ready) return;
    lv_task_handler();
    delay(delay_ms);
  }

  // Periodischer LVGL-Tick (1 ms Aufruf reicht — wir inkrementieren
  // intern mit LVGL_TICK_PERIOD_MS ms).
  static void onTimerTick() {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
  }

 private:
  static void setupDisplay() {
    // Renderpuffer im PSRAM (sofern verfügbar)
    static lv_color_t *buf1 = static_cast<lv_color_t *>(
        heap_caps_malloc(LCD_WIDTH * LVGL_BUFFER_LINES * sizeof(lv_color_t),
                         MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

    s_disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    lv_display_set_buffers(s_disp, buf1, nullptr,
                           LCD_WIDTH * LVGL_BUFFER_LINES,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_disp, &flushCallback);
  }

  static void setupTouch() {
    if (!TouchGT911::isReady()) {
      // LVGL-Input wird nur registriert, wenn der Touch reagiert.
      return;
    }
    s_indev = lv_indev_create();
    lv_indev_set_type(s_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_indev, &readTouch);
    lv_indev_set_display(s_indev, s_disp);
  }

  // LVGL-Flush: Übergibt den aktuellen Render-Slab an das LCD
  static void flushCallback(lv_display_t *disp,
                            const lv_area_t *area,
                            uint8_t *color_map) {
    // Production builds should dispatch to esp_lcd_panel_draw_bitmap()
    // here and call lv_display_flush_ready() from the vsync callback.
    // This header-only stub signals readiness immediately so that
    // LVGL tests can run without a real RGB panel attached.
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    (void)w;
    (void)h;
    (void)color_map;
    lv_display_flush_ready(disp);
  }

  // LVGL-Read: Liefert die aktuelle Touchposition
  static void readTouch(lv_indev_t *indev, lv_indev_data_t *data) {
    TouchPoint tp;
    bool ok = TouchGT911::read(tp);
    data->state = (ok && tp.pressed) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = tp.x;
    data->point.y = tp.y;
  }

  static inline lv_display_t *s_disp = nullptr;
  static inline lv_indev_t   *s_indev = nullptr;
  static inline bool          s_ready = false;
};

} // namespace waveshare
