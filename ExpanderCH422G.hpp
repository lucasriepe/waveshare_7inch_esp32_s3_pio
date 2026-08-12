#pragma once

// =====================================================================
//  ExpanderCH422G – I/O-Expander-Treiber (Backlight, Touch-RST, USB_SEL)
//  ----------------------------------------------------------------------
//  Hardware: CH422G mit zwei I2C-Adressen
//    - 0x24 → Konfigurationsregister (0x01 = Ausgabemodus)
//    - 0x38 → Datenausgabe (Backlight, Touch-RST, USB_SEL)
//
//  Werte stammen direkt aus 08_lvgl_Porting/main/waveshare_rgb_lcd_port.c
//  und werden dort beim LCD-/TWAI-/SD-Setup verwendet.
// =====================================================================

#include "BoardConfig.hpp"
#include "I2CBus.hpp"

namespace waveshare {

class ExpanderCH422G {
 public:
  // Initialisiert den Expander (einmalig). Setzt ihn auf Output-Modus.
  static bool begin() {
    if (s_ready) return true;
    // Konfigurationsregister: 0x01 → alle Ausgänge aktiv
    bool ok = I2CBus::writeByte(I2C_EXPANDER_CFG_ADDR, 0x01);
    s_ready = ok;
    return ok;
  }

  // Backlight an (0x1E entspricht der Originalimplementierung)
  static bool setBacklight(bool on) {
    if (!s_ready && !begin()) return false;
    return I2CBus::writeByte(I2C_EXPANDER_DATA_ADDR,
                             on ? CH422G_BL_ON : CH422G_BL_OFF);
  }

  // Touch-Reset-Sequenz: Preamble → Aux-GPIO low → Datenleitung low → delay → aux high
  static void pulseTouchReset(uint32_t delay_ms = 10) {
    if (!s_ready && !begin()) return;
    // Touch-Reset-Sequenz (gespiegelt aus waveshare_rgb_lcd_port.c)
    pinMode(PIN_TOUCH_AUX, OUTPUT);
    digitalWrite(PIN_TOUCH_AUX, LOW);
    I2CBus::writeByte(I2C_EXPANDER_DATA_ADDR, CH422G_TOUCH_RST_PRE);
    delay(delay_ms);
    I2CBus::writeByte(I2C_EXPANDER_DATA_ADDR, CH422G_TOUCH_RST_ASSERT);
    digitalWrite(PIN_TOUCH_AUX, HIGH);
  }

  // USB-Select high → GPIO19/20 werden auf CAN TX/RX geroutet
  static bool selectUsbHigh() {
    if (!s_ready && !begin()) return false;
    return I2CBus::writeByte(I2C_EXPANDER_DATA_ADDR, CH422G_USB_SEL_HIGH);
  }

  static bool selectUsbLow() {
    if (!s_ready && !begin()) return false;
    return I2CBus::writeByte(I2C_EXPANDER_DATA_ADDR, 0x00);
  }

  static bool isReady() { return s_ready; }

 private:
  static inline bool s_ready = false;
};

} // namespace waveshare
