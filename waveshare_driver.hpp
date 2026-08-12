#pragma once

// =====================================================================
//  waveshare_driver – Sammel-Header für das Waveshare-Board
//  ----------------------------------------------------------------------
//  Vereinfacht den Import: einmal
//      #include <waveshare_driver/waveshare_driver.hpp>
//  und alle Einzelmodule sind verfügbar.
// =====================================================================

#include "waveshare_driver/BoardConfig.hpp"
#include "waveshare_driver/I2CBus.hpp"
#include "waveshare_driver/ExpanderCH422G.hpp"
#include "waveshare_driver/TouchGT911.hpp"
#include "waveshare_driver/LcdPanel.hpp"
#include "waveshare_driver/SdCard.hpp"
#include "waveshare_driver/TwaiBus.hpp"
#include "waveshare_driver/AdcSensor.hpp"
#include "waveshare_driver/UartEcho.hpp"

namespace waveshare_driver {
// Aliasse für komfortableren Aufruf
namespace ws = ::waveshare;
}
