#pragma once

// =====================================================================
//  AdcSensor – analoger Eingang am GPIO6 (ADC1_CH5)
//  ----------------------------------------------------------------------
//  Wertet das boardport-04_Sensor_AD-Beispiel als wiederverwendbares
//  Arduino-API nach. Liefert Rohwert und mV (lineare Schätzung).
//
//  Auf aktuellen Arduino-ESP32-Versionen (3.x+) ist der
//  `driver/adc.h`-Legacy-Header deprecated. Wir verwenden daher die
//  generische Arduino-API (`analogRead` / `analogReadMilliVolts`),
//  die auf allen ESP32-Varianten verfügbar ist.
// =====================================================================

#include <Arduino.h>
#include "BoardConfig.hpp"

namespace waveshare {

class AdcSensor {
 public:
  // Bringt den ADC online. Bei ESP32-S3 ist analogRead() immer
  // verfügbar, daher genügt diese Idempotenz-Funktion.
  static bool begin() {
    if (s_ready) return true;
#if defined(ARDUINO_ARCH_ESP32)
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_ADC_CHANNEL, ADC_11db);
#endif
    s_ready = true;
    return s_ready;
  }

  static bool isReady() { return s_ready; }

  // Roher 12-bit-Wert (0..4095 bei V_REF ~ 3.3 V)
  static int readRaw() {
    if (!s_ready) begin();
    return analogRead(PIN_ADC_CHANNEL);
  }

  // Millivolt-Schätzung. Bevorzugt die eFuse-/Kalibrierungs-API von
  // ESP32, fällt andernfalls auf eine lineare Skalierung zurück.
  static int readMillivolts() {
    int raw = readRaw();
    if (raw <= 0) return 0;
#if defined(ARDUINO_ARCH_ESP32) && defined(analogReadMilliVolts)
    // Auf ESP32-S3 / ESP32 liefert analogReadMilliVolts() bereits
    // eine (kalibrierungsgestützte) mV-Schätzung.
    return static_cast<int>(analogReadMilliVolts(PIN_ADC_CHANNEL));
#else
    // Lineare Annahme 0..4095 ⇒ 0..3300 mV
    return static_cast<int>((int64_t)raw * 3300 / 4095);
#endif
  }

  // Kompatibilität zu boardport-Schleife (alle 1000 ms).
  static bool runSamplingLoop(uint32_t period_ms = ADC_SAMPLE_PERIOD_MS,
                              bool (*consumer)(int mv) = nullptr) {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (now - last < period_ms) return false;
    last = now;
    int mv = readMillivolts();
    if (consumer) return consumer(mv);
    return true;
  }

 private:
  static inline bool s_ready = false;
};

} // namespace waveshare