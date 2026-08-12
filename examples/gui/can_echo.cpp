// =====================================================================
//  can_echo.cpp — TWAI echo (RX → TX) using TwaiBus
//  ----------------------------------------------------------------------
//  Demonstrates how to drive the CAN bus with the driver wrapper. Every
//  received frame is mirrored back to the bus so a second node can see
//  its own traffic. Both standard and extended identifiers are supported.
//
//  Integration in src/main.cpp:
//
//      #include "examples/gui/can_echo.cpp"
//
//      void setup() {
//          can_echo_begin(/*bitrate=*/500000);
//      }
//
//      void loop() {
//          can_echo_tick();
//          LcdPanel::tick(5);
//      }
//
//  Hardware note: GPIO0 / GPIO2 are shared with USB through the FSUSB42UMX
//  mux. TwaiBus::begin() routes the CAN signals automatically — do not
//  open the USB serial port while the bus is running.
// =====================================================================

#ifndef WAVESHARE_EXAMPLE_CAN_ECHO_HPP
#define WAVESHARE_EXAMPLE_CAN_ECHO_HPP

#include <Arduino.h>
#include <waveshare_driver/waveshare_driver.hpp>

using namespace waveshare;

namespace waveshare_examples {

static bool s_ready = false;

static bool can_echo_begin(uint32_t bitrate = 500000,
                           TwaiRole role = TwaiRole::Normal) {
  s_ready = TwaiBus::begin(bitrate, role);
  return s_ready;
}

static void can_echo_end() {
  TwaiBus::end();
  s_ready = false;
}

// Drain all frames currently in the RX queue and mirror them on TX.
static void can_echo_tick() {
  if (!s_ready) return;

  // Block briefly (1 tick) to allow RX to populate the queue.
  TwaiMessage msg;
  while (TwaiBus::receive(msg, /*timeout_ms=*/0)) {
    Serial.printf("[can_echo] RX id=0x%03X dlc=%u ext=%u rtr=%u data=",
                  (unsigned)msg.identifier,
                  (unsigned)msg.data_length_code,
                  msg.ext ? 1u : 0u,
                  msg.rtr ? 1u : 0u);
    for (uint8_t i = 0; i < msg.data_length_code; ++i) {
      Serial.printf("%02X ", msg.data[i]);
    }
    Serial.println();

    // Reflect the frame so other nodes can see their own traffic.
    TwaiBus::transmit(msg.identifier, msg.data, msg.data_length_code, msg.rtr);
  }
}

} // namespace waveshare_examples

using waveshare_examples::can_echo_begin;
using waveshare_examples::can_echo_end;
using waveshare_examples::can_echo_tick;

#endif // WAVESHARE_EXAMPLE_CAN_ECHO_HPP