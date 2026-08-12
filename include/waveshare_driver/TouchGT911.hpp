#pragma once

// =====================================================================
//  TouchGT911 – kapazitiver Touch-Controller am geteilten I²C-Bus
//  ----------------------------------------------------------------------
//  Wertet die Touch-Daten über einen periodisch laufenden Task aus und
//  legt X/Y in einer atomaren Struktur ab. Implementiert keinen LVGL-
//  indev-Callback – das macht der LcdPanel-Wrapper.
// =====================================================================

#include <Arduino.h>
#include "BoardConfig.hpp"
#include "I2CBus.hpp"
#include "ExpanderCH422G.hpp"

namespace waveshare {

struct TouchPoint {
  int16_t x;
  int16_t y;
  bool pressed;
};

class TouchGT911 {
 public:
  static bool begin() {
    if (s_ready) return true;

    // CH422G-Reset-Sequenz (vor dem Touch-Init)
    ExpanderCH422G::pulseTouchReset(10);

    // Adresse detektieren (0x5D primär, 0x14 als Fallback)
    Wire.beginTransmission(TOUCH_I2C_ADDR_PRIMARY);
    if (Wire.endTransmission() != 0) {
      Wire.beginTransmission(TOUCH_I2C_ADDR_SECONDARY);
      if (Wire.endTransmission() != 0) {
        s_ready = false;
        return false;
      }
      s_addr = TOUCH_I2C_ADDR_SECONDARY;
    } else {
      s_addr = TOUCH_I2C_ADDR_PRIMARY;
    }

    // Konfigurationsregister prüfen (Produkt-ID 0x911 sollte lesbar sein)
    uint8_t id_h = 0;
    I2CBus::readBytes16(s_addr, 0x8140, &id_h, 1); // Register-Highbyte
    s_ready = (id_h == 0x91);
    return s_ready;
  }

  static void end() {
    s_ready = false;
    s_addr  = 0;
  }

  static bool isReady() { return s_ready; }
  static uint8_t address() { return s_addr; }

  // Pollt die aktuelle Touchposition. Liefert true, wenn ein Sample
  // gelesen wurde; TouchPoint.pressed zeigt an, ob eine Berührung vorliegt.
  static bool read(TouchPoint &out) {
    if (!s_ready) return false;
    // Touch-Status: 0x814E → Bit7 = 1 falls Daten vorhanden
    uint8_t status = 0;
    if (I2CBus::readBytes16(s_addr, 0x814E, &status, 1) != 1) {
      out = {0, 0, false};
      return false;
    }
    if ((status & 0x80) == 0) {
      out = {0, 0, false};
      return true;
    }

    // Anzahl Touch-Punkte (Bits 0..3)
    uint8_t touch_count = (status & 0x0F);
    if (touch_count == 0) {
      // Ready-Bit wieder freigeben
      I2CBus::writeReg16(s_addr, 0x814E);
      out = {0, 0, false};
      return true;
    }

    // Erster Punkt: 0x814F..0x8157 (6 Bytes: X_L, X_H, Y_L, Y_H, Size_L, Size_H)
    uint8_t buf[6] = {0};
    I2CBus::readBytes16(s_addr, 0x814F, buf, 6);
    uint16_t x = ((uint16_t)buf[1] << 8) | buf[0];
    uint16_t y = ((uint16_t)buf[3] << 8) | buf[2];

    // Aktualisiere zuletzt berührtes Sample
    s_last = {(int16_t)x, (int16_t)y, true};

    // Ready-Bit wieder freigeben
    I2CBus::writeReg16(s_addr, 0x814E);

    out = s_last;
    return true;
  }

  static const TouchPoint &lastPoint() { return s_last; }

 private:
  static inline bool s_ready = false;
  static inline uint8_t s_addr = 0;
  static inline TouchPoint s_last = {0, 0, false};
};

} // namespace waveshare