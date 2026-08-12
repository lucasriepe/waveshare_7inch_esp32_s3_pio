#pragma once

// =====================================================================
//  TwaiBus – TWAI (CAN) Wrapper für Waveshare ESP32-S3 Touch LCD 7
//  ----------------------------------------------------------------------
//  Pins: TX=GPIO0, RX=GPIO2 (boardport-Konfiguration).
//  Vor dem Starten muss CH422G->USB_SEL HIGH geschaltet werden, damit
//  GPIO19/20 an die FSUSB42UMX-MUX durchgereicht werden.
// =====================================================================

#include <Arduino.h>
#include "BoardConfig.hpp"
#include "I2CBus.hpp"
#include "ExpanderCH422G.hpp"

// Vor Arduino-esp32 3.x: CAN.h. Ab 3.x: driver/twai.h.
#if __has_include(<driver/twai.h>)
  #include <driver/twai.h>
  #define WAVESHARE_USE_NEW_TWAI 1
#elif __has_include(<CAN.h>)
  #include <CAN.h>
  #define WAVESHARE_USE_NEW_TWAI 0
#endif

namespace waveshare {

struct TwaiMessage {
  uint32_t identifier;
  uint8_t  data[8];
  uint8_t  data_length_code;
  bool     ext;
  bool     rtr;
};

enum class TwaiRole : uint8_t {
  Normal = 0,        // boardport: TWAI_MODE_NO_ACK
  ListenOnly = 1,    // boardport: 07_TWAIreceive
  NoAck     = 2,     // boardport: 06_TWAItransmit
};

class TwaiBus {
 public:
  static bool begin(uint32_t bitrate = TWAI_BITRATE_500KBPS,
                    TwaiRole role = TwaiRole::Normal) {
    if (s_ready) return true;

    if (!I2CBus::begin(Wire, PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_HZ)) {
      return false;
    }
    if (!ExpanderCH422G::begin()) {
      return false;
    }
    // USB-Select auf CAN schalten
    ExpanderCH422G::selectUsbHigh();

#if WAVESHARE_USE_NEW_TWAI
    static twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)PIN_TWAI_TX,
                                    (gpio_num_t)PIN_TWAI_RX,
                                    TWAI_MODE_NORMAL);
    static twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    static twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    g_config.tx_io = (gpio_num_t)PIN_TWAI_TX;
    g_config.rx_io = (gpio_num_t)PIN_TWAI_RX;
    switch (role) {
      case TwaiRole::ListenOnly: g_config.mode = TWAI_MODE_LISTEN_ONLY; break;
      case TwaiRole::NoAck:      g_config.mode = TWAI_MODE_NO_ACK;      break;
      default:                   g_config.mode = TWAI_MODE_NORMAL;       break;
    }

    if (bitrate == TWAI_BITRATE_50KBPS) {
      t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_50KBITS();
    }

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
      return false;
    }
    if (twai_start() != ESP_OK) {
      return false;
    }
#else
    CAN.setPins(PIN_TWAI_TX, PIN_TWAI_RX);
    CAN.begin(bitrate);
#endif

    s_ready = true;
    return true;
  }

  static void end() {
    if (!s_ready) return;
#if WAVESHARE_USE_NEW_TWAI
    twai_stop();
    twai_driver_uninstall();
#else
    CAN.end();
#endif
    ExpanderCH422G::selectUsbLow();
    s_ready = false;
  }

  static bool isReady() { return s_ready; }

  // Sendet einen Frame. identifier < 0x800 = Standard, sonst Extended.
  static bool transmit(uint32_t identifier, const uint8_t *data, uint8_t len,
                       bool rtr = false) {
    if (!s_ready || len > 8) return false;

#if WAVESHARE_USE_NEW_TWAI
    twai_message_t m = {};
    m.identifier = identifier;
    m.data_length_code = len;
    m.extd = (identifier > 0x7FF);
    m.rtr = rtr;
    if (len > 0) memcpy(m.data, data, len);

    esp_err_t err = twai_transmit(&m, pdMS_TO_TICKS(1000));
    return err == ESP_OK;
#else
    CAN.beginPacket((identifier > 0x7FF) ? 1 : 0, identifier, rtr, len);
    for (uint8_t i = 0; i < len; ++i) CAN.write(data[i]);
    return CAN.endPacket();
#endif
  }

  // Empfängt einen Frame. timeout_ms = 0 → kein Blocken.
  static bool receive(TwaiMessage &out, uint32_t timeout_ms = 0) {
    if (!s_ready) return false;

#if WAVESHARE_USE_NEW_TWAI
    twai_message_t m = {};
    esp_err_t err = twai_receive(&m, pdMS_TO_TICKS(timeout_ms));
    if (err != ESP_OK) return false;
    out.identifier = m.identifier;
    out.data_length_code = m.data_length_code;
    out.ext = m.extd;
    out.rtr = m.rtr;
    memset(out.data, 0, sizeof(out.data));
    if (m.data_length_code > 0) {
      memcpy(out.data, m.data, m.data_length_code);
    }
    return true;
#else
    int packetSize = CAN.parsePacket();
    if (packetSize == 0) {
      delay(timeout_ms);
      return false;
    }
    out.identifier = CAN.packetId();
    out.ext = false;
    out.rtr = false;
    out.data_length_code = packetSize;
    memset(out.data, 0, sizeof(out.data));
    for (uint8_t i = 0; i < packetSize && i < 8; ++i) {
      out.data[i] = CAN.read();
    }
    return true;
#endif
  }

  // Vordefinierte Testbotschaft (boardport 06_TWAItransmit)
  static bool transmitTestFrame() {
    uint8_t data[8];
    for (uint8_t i = 0; i < 8; ++i) data[i] = i;
    return transmit(0x0F6, data, 8, false);
  }

  // Periodisches Senden (boardport-Default 1000 ms)
  static bool runTransmitterLoop(uint32_t period_ms = TWAI_TRANSMIT_RATE_MS) {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (now - last < period_ms) return false;
    last = now;
    return transmitTestFrame();
  }

 private:
  static inline bool s_ready = false;
};

} // namespace waveshare
