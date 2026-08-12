#pragma once

// =====================================================================
//  UartEcho – UART-Helper
//  ----------------------------------------------------------------------
//  Kompatibel zu boardport/02_RS485_Test und /05_UART_Test.
//  Default: UART2, 115200, 8N1, TX=4, RX=5 (S3-Default im boardport).
//  Optionaler Buffer-Echo-Task im RAM.
// =====================================================================

#include <Arduino.h>
#include "BoardConfig.hpp"

namespace waveshare {

class UartEcho {
 public:
  static bool begin(HardwareSerial &serial = Serial2,
                    uint32_t baud = UART_BAUD,
                    int8_t tx = UART_PIN_TX,
                    int8_t rx = UART_PIN_RX,
                    uint32_t stack_words = UART_TASK_STACK_SIZE) {
    if (s_ready) return true;
    s_serial = &serial;
    serial.begin(baud, SERIAL_8N1, rx, tx);
    s_task_running = true;
    if (s_task_handle == nullptr) {
      xTaskCreateUniversal(&echoTask, "waveshare_uart_echo",
                           stack_words, nullptr, 10,
                           &s_task_handle,
                           ARDUINO_RUNNING_CORE ? 0 : 1);
    }
    s_ready = true;
    return true;
  }

  static void end() {
    if (!s_ready) return;
    if (s_serial) {
      s_serial->end();
    }
    if (s_task_handle) {
      vTaskDelete(s_task_handle);
      s_task_handle = nullptr;
    }
    s_ready = false;
  }

  static bool isReady() { return s_ready; }
  static HardwareSerial &serial() { return *s_serial; }

 private:
  static void echoTask(void *arg) {
    (void)arg;
    static uint8_t buf[UART_BUF_SIZE];
    while (true) {
      if (s_serial && s_serial->available()) {
        size_t got = s_serial->readBytes(buf, UART_BUF_SIZE);
        s_serial->write(buf, got);
      }
      delay(10);
    }
  }

  static inline HardwareSerial *s_serial = nullptr;
  static inline TaskHandle_t s_task_handle = nullptr;
  static inline bool s_ready = false;
  static inline volatile bool s_task_running = false;
};

} // namespace waveshare
