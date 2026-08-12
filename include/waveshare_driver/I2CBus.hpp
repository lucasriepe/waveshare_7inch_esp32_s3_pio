#pragma once

// =====================================================================
//  I2CBus – shared I²C manager for touch / backlight / SD-CS / TWAI
//  ----------------------------------------------------------------------
//  On the boardport project every peripheral with CH422G connectivity
//  (SD, TWAI, RGB backlight, touch reset) shares the same I2C_NUM_0 bus
//  on SDA=8 / SCL=9 at 400 kHz.
//
//  All helpers below are safe to call before begin() — they will return
//  false / 0 instead of dereferencing a null Wire handle.
// =====================================================================

#include <Arduino.h>
#include <Wire.h>
#include "BoardConfig.hpp"

namespace waveshare {

class I2CBus {
 public:
  // Bring up the shared bus exactly once. Idempotent.
  static bool begin(TwoWire &wire = Wire,
                    int8_t sda = PIN_I2C_SDA,
                    int8_t scl = PIN_I2C_SCL,
                    uint32_t clock_hz = I2C_CLOCK_HZ) {
    if (s_initialized && s_wire != nullptr) return true;
    s_wire = &wire;
    wire.begin(sda, scl, clock_hz);
    s_initialized = true;
    return true;
  }

  static TwoWire *handle() { return s_wire; }
  static bool isInitialized() { return s_initialized; }

  // Safe accessor: returns the underlying Wire if the bus is up,
  // otherwise nullptr. Callers must check the result.
  static TwoWire *safeWire() {
    return (s_initialized && s_wire != nullptr) ? s_wire : nullptr;
  }

  // Helper: write one byte to a peripheral address (NO-ACK tolerant).
  static bool writeByte(uint8_t addr, uint8_t value) {
    TwoWire *bus = safeWire();
    if (!bus) return false;
    bus->beginTransmission(addr);
    bus->write(value);
    return bus->endTransmission() == 0;
  }

  // Helper: write a 16-bit register address + one data byte
  // (used by GT911 and similar devices).
  static bool writeReg16u8(uint8_t addr, uint16_t reg, uint8_t value) {
    TwoWire *bus = safeWire();
    if (!bus) return false;
    bus->beginTransmission(addr);
    bus->write((uint8_t)(reg >> 8));
    bus->write((uint8_t)(reg & 0xFF));
    bus->write(value);
    return bus->endTransmission() == 0;
  }

  // Helper: write a 16-bit register address only (no data byte),
  // e.g. clearing the GT911 status register.
  static bool writeReg16(uint8_t addr, uint16_t reg) {
    return writeReg16u8(addr, reg, 0);
  }

  // Helper: read N bytes from an 8-bit register.
  static size_t readBytes(uint8_t addr, uint8_t reg, uint8_t *out, uint8_t len) {
    TwoWire *bus = safeWire();
    if (!bus) return 0;
    bus->beginTransmission(addr);
    bus->write(reg);
    if (bus->endTransmission(false) != 0) return 0;
    size_t got = bus->requestFrom((int)addr, (int)len);
    for (size_t i = 0; i < got && i < len; ++i) out[i] = bus->read();
    return got;
  }

  // Helper: read N bytes from a 16-bit register.
  static size_t readBytes16(uint8_t addr, uint16_t reg, uint8_t *out, uint8_t len) {
    TwoWire *bus = safeWire();
    if (!bus) return 0;
    bus->beginTransmission(addr);
    bus->write((uint8_t)(reg >> 8));
    bus->write((uint8_t)(reg & 0xFF));
    if (bus->endTransmission(false) != 0) return 0;
    size_t got = bus->requestFrom((int)addr, (int)len);
    for (size_t i = 0; i < got && i < len; ++i) out[i] = bus->read();
    return got;
  }

 private:
  static inline TwoWire *s_wire = nullptr;
  static inline bool s_initialized = false;
};

} // namespace waveshare