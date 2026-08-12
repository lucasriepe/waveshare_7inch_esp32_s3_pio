// =====================================================================
//  sd_logger.cpp — periodic CSV-style logging to the SD card
//  ----------------------------------------------------------------------
//  Demonstrates how to use SdCard together with the ADC driver to log
//  sensor voltages to /sdcard/log.csv once per second. The file is kept
//  open across writes for efficiency; reopen is automatic on truncation
//  or mount failure.
//
//  Integration in src/main.cpp:
//
//      #include "examples/gui/sd_logger.cpp"
//
//      void setup() {
//          AdcSensor::begin();
//          SdCard::begin(false);
//          sd_logger_begin();
//      }
//
//      void loop() {
//          sd_logger_tick();
//          LcdPanel::tick(5);
//      }
//
//  Each row contains: <millis>, <voltage_mV>.
// =====================================================================

#ifndef WAVESHARE_EXAMPLE_SD_LOGGER_HPP
#define WAVESHARE_EXAMPLE_SD_LOGGER_HPP

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <waveshare_driver/waveshare_driver.hpp>

using namespace waveshare;

namespace waveshare_examples {

static constexpr const char *kLogPath      = "/sdcard/log.csv";
static constexpr const char *kHeader       = "sample_ms,voltage_mv\n";
static constexpr uint32_t     kPeriodMs    = 1000;
static constexpr size_t       kLineMaxLen  = 64;

static File     s_file;
static bool     s_ready   = false;
static uint32_t s_last_ms = 0;

static bool sd_logger_open() {
  if (!SdCard::isMounted()) return false;

  // Create the file with a header line if it does not exist yet.
  if (!SD.exists(kLogPath)) {
    File first = SD.open(kLogPath, FILE_WRITE);
    if (!first) return false;
    first.print(kHeader);
    first.close();
  }

  s_file = SD.open(kLogPath, FILE_APPEND);
  s_ready = static_cast<bool>(s_file);
  return s_ready;
}

static bool sd_logger_begin() {
  s_ready = false;
  if (s_file) {
    s_file.close();
  }
  return sd_logger_open();
}

static void sd_logger_close() {
  if (s_file) {
    s_file.flush();
    s_file.close();
  }
  s_ready = false;
}

static void sd_logger_tick() {
  if (!s_ready && !sd_logger_open()) return;

  const uint32_t now = millis();
  if (now - s_last_ms < kPeriodMs) return;
  s_last_ms = now;

  // Auto-reopen if the file handle has been invalidated (e.g. card removed).
  if (!s_file) {
    if (!sd_logger_open()) return;
  }

  const int mv = AdcSensor::readMillivolts();
  char line[kLineMaxLen];
  const int n = snprintf(line, sizeof(line), "%lu,%d\n",
                         (unsigned long)now, mv);
  if (n > 0) {
    s_file.print(line);
    // Periodic flush keeps the file consistent across power cycles.
    static uint32_t s_last_flush = 0;
    if (now - s_last_flush >= 5000) {
      s_last_flush = now;
      s_file.flush();
    }
    Serial.printf("[sd_logger] %s", line);
  }
}

} // namespace waveshare_examples

using waveshare_examples::sd_logger_begin;
using waveshare_examples::sd_logger_tick;
using waveshare_examples::sd_logger_close;

#endif // WAVESHARE_EXAMPLE_SD_LOGGER_HPP