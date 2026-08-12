#pragma once

// =====================================================================
//  I2CBus – gemeinsamer I²C-Manager für Touch / Backlight / SD-CS / TWAI
//  ----------------------------------------------------------------------
//  Im boardport/-Projekt teilen sich alle Peripheriegeräte mit
//  CH422G-Anbindung (SD, TWAI, RGB-Backlight, Touch-Reset) den
//  gleichen I2C_NUM_0-Bus mit SDA=8, SCL=9, 400 kHz.
// =====================================================================

#include <Arduino.h>
#include <Wire.h>
#include "BoardConfig.hpp"

namespace waveshare {

class I2CBus {
 public:
  // Beginnt den geteilten I²C-Bus exakt einmal. Idempotent.
  static bool begin(TwoWire &wire = Wire,
                    int8_t sda = PIN_I2C_SDA,
                    int8_t scl = PIN_I2C_SCL,
                    uint32_t clock_hz = I2C_CLOCK_HZ) {
    if (s_initialized) return true;
    s_wire = &wire;
    wire.begin(sda, scl, clock_hz);
    s_initialized = true;
    return true;
  }

  static TwoWire &wire() { return *s_wire; }
  static bool isInitialized() { return s_initialized; }

  // Hilfsfunktion: Schreibbyte an eine Expander-Adresse (NO-ACK tolerant)
  static bool writeByte(uint8_t addr, uint8_t value) {
    wire().beginTransmission(addr);
    wire().write(value);
    return wire().endTransmission() == 0;
  }

  // Hilfsfunktion: Write 16-bit-Register + 1 Data-Byte (für GT911 etc.)
  static bool writeReg16u8(uint8_t addr, uint16_t reg, uint8_t value) {
    wire().beginTransmission(addr);
    wire().write((uint8_t)(reg >> 8));   // Register-Highbyte zuerst
    wire().write((uint8_t)(reg & 0xFF)); // Register-Lowbyte
    wire().write(value);
    return wire().endTransmission() == 0;
  }

  // Hilfsfunktion: Write 16-bit-Register ohne Data (z.B. Status zurücksetzen)
  static bool writeReg16(uint8_t addr, uint16_t reg) {
    return writeReg16u8(addr, reg, 0);
  }

  // Hilfsfunktion: Read-N-Bytes ab einem 8-bit-Register
  static size_t readBytes(uint8_t addr, uint8_t reg, uint8_t *out, uint8_t len) {
    wire().beginTransmission(addr);
    wire().write(reg);
    if (wire().endTransmission(false) != 0) return 0;
    size_t got = wire().requestFrom((int)addr, (int)len);
    for (size_t i = 0; i < got && i < len; ++i) out[i] = wire().read();
    return got;
  }

  // Hilfsfunktion: Read-N-Bytes ab einem 16-bit-Register (für GT911)
  static size_t readBytes16(uint8_t addr, uint16_t reg, uint8_t *out, uint8_t len) {
    wire().beginTransmission(addr);
    wire().write((uint8_t)(reg >> 8));
    wire().write((uint8_t)(reg & 0xFF));
    if (wire().endTransmission(false) != 0) return 0;
    size_t got = wire().requestFrom((int)addr, (int)len);
    for (size_t i = 0; i < got && i < len; ++i) out[i] = wire().read();
    return got;
  }

 private:
  static inline TwoWire *s_wire = nullptr;
  static inline bool s_initialized = false;
};

} // namespace waveshare