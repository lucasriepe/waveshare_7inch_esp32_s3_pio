#pragma once

// =====================================================================
//  Waveshare ESP32-S3 Touch LCD 7 - hardware configuration
//  ----------------------------------------------------------------------
//  All values are `constexpr`, so they are visible to the compiler without
//  any Kconfig / SDKCONFIG plumbing. The pin map, peripheral constants
//  and timing numbers are taken verbatim from the `boardport/`
//  reference examples:
//
//    08_lvgl_Porting  — RGB-LCD + LVGL
//    03_SD_Test       — SD card over SPI
//    06_TWAItransmit  — CAN TX
//    07_TWAIreceive   — CAN RX
//    04_Sensor_AD     — ADC sensor
//    02_RS485_Test    — UART (same as 05_UART_Test)
//    01_I2C_Test      — I²C tools
//
//  The chip-level constants at the top (`MCU_*`, `MEMORY_*`) match the
//  ESP32-S3-WROOM-1 datasheet (Xtensa LX7 dual-core, 240 MHz, 512 KB
//  internal SRAM, up to 8 MB PSRAM, up to 16 MB Quad SPI flash).
// =====================================================================

#include <stdint.h>

namespace waveshare {

// ---------------------------------------------------------------------
//  Chip-level constants — derived from the ESP32-S3 datasheet
// ---------------------------------------------------------------------
static constexpr uint32_t MCU_ARCH                 = 32;          // bits
static constexpr const char *MCU_NAME               = "ESP32-S3";
static constexpr uint32_t MCU_CORE_COUNT           = 2;
static constexpr uint32_t MCU_CPU_FREQ_HZ          = 240000000;   // 240 MHz
static constexpr uint32_t MCU_ROM_BYTES            = 384 * 1024;  // 384 KB
static constexpr uint32_t MCU_INTERNAL_SRAM_BYTES  = 512 * 1024;  // 512 KB
static constexpr uint32_t MCU_RTC_SRAM_BYTES       = 16  * 1024;  // 16 KB
static constexpr uint32_t MCU_PSRAM_MAX_BYTES      = 8   * 1024 * 1024; // 8 MB
static constexpr uint32_t MCU_FLASH_MAX_BYTES      = 16  * 1024 * 1024; // 16 MB

// PSRAM is fitted on the Waveshare 7-inch board (ESP32-S3-WROOM-1 N16R8).
static constexpr bool     HAS_PSRAM                = true;
static constexpr uint32_t PSRAM_SIZE_BYTES         = 8 * 1024 * 1024; // 8 MB
static constexpr uint32_t FLASH_SIZE_BYTES         = 16 * 1024 * 1024; // 16 MB

// Memory budgets for LVGL (see LcdPanel::setupDisplay). Internal SRAM is
// precious (512 KB total, ~320 KB free after Arduino + FreeRTOS), so
// the render buffer must live in PSRAM whenever it is available.
static constexpr uint32_t LVGL_BUFFER_INTERNAL_BUDGET_BYTES = 64 * 1024;   // 64 KB cap
static constexpr uint32_t LVGL_BUFFER_PSRAM_BUDGET_BYTES    = 1024 * 1024; // 1 MB cap

// ---------------------------------------------------------------------
//  Display geometry
// ---------------------------------------------------------------------
static constexpr uint16_t LCD_WIDTH                = 800;
static constexpr uint16_t LCD_HEIGHT               = 480;
static constexpr uint32_t LCD_PIXEL_CLOCK_HZ       = 16u * 1000u * 1000u; // 16 MHz
static constexpr uint8_t  LCD_BITS_PER_PIXEL       = 16;

// RGB timing (mirrors EXAMPLE_RGB_HSYNC/VSYNC in the boardport project)
static constexpr uint32_t LCD_HSYNC_PULSE_WIDTH    = 4;
static constexpr uint32_t LCD_HSYNC_BACK_PORCH     = 8;
static constexpr uint32_t LCD_HSYNC_FRONT_PORCH    = 8;
static constexpr uint32_t LCD_VSYNC_PULSE_WIDTH    = 4;
static constexpr uint32_t LCD_VSYNC_BACK_PORCH     = 8;
static constexpr uint32_t LCD_VSYNC_FRONT_PORCH    = 8;
static constexpr uint32_t LCD_PCLK_ACTIVE_NEG      = 1;
static constexpr uint32_t LCD_SRAM_ALIGN           = 4;
static constexpr uint32_t LCD_PSRAM_ALIGN          = 64;
static constexpr uint32_t LCD_BB_LINES             = 10; // bounce buffer

// ---------------------------------------------------------------------
//  RGB data pins (16-bit parallel)
// ---------------------------------------------------------------------
static constexpr int8_t PIN_LCD_HSYNC = 46;
static constexpr int8_t PIN_LCD_VSYNC = 3;
static constexpr int8_t PIN_LCD_DE    = 5;
static constexpr int8_t PIN_LCD_PCLK  = 7;
static constexpr int8_t PIN_LCD_DISP  = -1;

static constexpr int8_t PIN_LCD_D0  = 14;
static constexpr int8_t PIN_LCD_D1  = 38;
static constexpr int8_t PIN_LCD_D2  = 18;
static constexpr int8_t PIN_LCD_D3  = 17;
static constexpr int8_t PIN_LCD_D4  = 10;
static constexpr int8_t PIN_LCD_D5  = 39;
static constexpr int8_t PIN_LCD_D6  = 0;
static constexpr int8_t PIN_LCD_D7  = 45;
static constexpr int8_t PIN_LCD_D8  = 48;
static constexpr int8_t PIN_LCD_D9  = 47;
static constexpr int8_t PIN_LCD_D10 = 21;
static constexpr int8_t PIN_LCD_D11 = 1;
static constexpr int8_t PIN_LCD_D12 = 2;
static constexpr int8_t PIN_LCD_D13 = 42;
static constexpr int8_t PIN_LCD_D14 = 41;
static constexpr int8_t PIN_LCD_D15 = 40;

// ---------------------------------------------------------------------
//  I2C bus (shared)
// ---------------------------------------------------------------------
//  Identical in 03_SD_Test, 06_TWAItransmit and 08_lvgl_Porting:
//    SDA = 8, SCL = 9, port 0, 400 kHz, CH422G expander @ 0x24/0x38
static constexpr int8_t   PIN_I2C_SDA                = 8;
static constexpr int8_t   PIN_I2C_SCL                = 9;
static constexpr uint32_t I2C_CLOCK_HZ               = 400 * 1000;
static constexpr uint8_t  I2C_EXPANDER_CFG_ADDR      = 0x24;
static constexpr uint8_t  I2C_EXPANDER_DATA_ADDR     = 0x38;

// CH422G output byte constants (see waveshare_rgb_lcd_port.c)
static constexpr uint8_t CH422G_TOUCH_RST_PRE        = 0x2C;
static constexpr uint8_t CH422G_TOUCH_RST_ASSERT     = 0x2E;
static constexpr uint8_t CH422G_BL_ON                = 0x1E;
static constexpr uint8_t CH422G_BL_OFF               = 0x1A;
static constexpr uint8_t CH422G_USB_SEL_HIGH         = 0x20; // routes GPIO0/2 → CAN

// Auxiliary GPIO used for the touch reset pulse (synchronised with GPIO4
// in the boardport reference).
static constexpr int8_t PIN_TOUCH_AUX = 4;

// ---------------------------------------------------------------------
//  Touch (GT911)
// ---------------------------------------------------------------------
//  The primary I2C address is 0x5D, the secondary 0x14 (selected by the
//  GT911 boot-strapping resistor on the panel).
static constexpr uint8_t  TOUCH_I2C_ADDR_PRIMARY     = 0x5D;
static constexpr uint8_t  TOUCH_I2C_ADDR_SECONDARY   = 0x14;
static constexpr uint16_t TOUCH_X_MAX                = LCD_WIDTH;
static constexpr uint16_t TOUCH_Y_MAX                = LCD_HEIGHT;

// ---------------------------------------------------------------------
//  SD card (SPI)
// ---------------------------------------------------------------------
static constexpr int8_t   PIN_SD_MOSI                = 35;
static constexpr int8_t   PIN_SD_MISO                = 37;
static constexpr int8_t   PIN_SD_CLK                 = 36;
static constexpr int8_t   PIN_SD_CS                  = 34;
static constexpr uint32_t SD_SPI_CLOCK_HZ            = 20 * 1000 * 1000; // SDMMC_FREQ_DEFAULT
static constexpr uint32_t SD_MAX_TRANSFER_SZ         = 4000;
static constexpr uint8_t  SD_MAX_FILES_OPEN          = 5;
static constexpr uint32_t SD_ALLOCATION_UNIT_SIZE    = 16 * 1024;
static constexpr const char *SD_MOUNT_POINT          = "/sdcard";

// ---------------------------------------------------------------------
//  TWAI / CAN
// ---------------------------------------------------------------------
static constexpr int8_t   PIN_TWAI_TX                = 0;
static constexpr int8_t   PIN_TWAI_RX                = 2;
static constexpr uint32_t TWAI_BITRATE_50KBPS        = 50000;
static constexpr uint32_t TWAI_BITRATE_500KBPS       = 500000;
static constexpr uint32_t TWAI_TRANSMIT_RATE_MS      = 1000;

// ---------------------------------------------------------------------
//  ADC (ADC1_CH5 on GPIO6)
// ---------------------------------------------------------------------
static constexpr int8_t   PIN_ADC_CHANNEL            = 6;     // GPIO6 = ADC1_CH5
static constexpr uint32_t ADC_SAMPLE_PERIOD_MS       = 1000;

// ---------------------------------------------------------------------
//  UART
// ---------------------------------------------------------------------
static constexpr uint32_t UART_BAUD                  = 115200;
static constexpr uint8_t  UART_PORT                  = 2;
static constexpr int8_t   UART_PIN_TX                = 4;
static constexpr int8_t   UART_PIN_RX                = 5;
static constexpr uint16_t UART_BUF_SIZE              = 1024;
static constexpr uint16_t UART_TASK_STACK_SIZE       = 2048;

// ---------------------------------------------------------------------
//  LVGL defaults
// ---------------------------------------------------------------------
static constexpr uint32_t LVGL_TICK_PERIOD_MS        = 2;
static constexpr uint32_t LVGL_TASK_MAX_DELAY_MS     = 500;
static constexpr uint32_t LVGL_TASK_MIN_DELAY_MS     = 10;
static constexpr uint32_t LVGL_TASK_STACK_SIZE       = 6 * 1024;
static constexpr uint8_t  LVGL_TASK_PRIORITY         = 2;
static constexpr int8_t   LVGL_TASK_CORE             = -1;
static constexpr uint16_t LVGL_BUFFER_LINES          = 100;

// LVGL heap: Arduino-ESP32 exposes ~320 KB of internal SRAM for the
// sketch; we reserve 96 KB for LVGL's memory manager (objects, fonts).
// The render buffer sits in PSRAM (see LcdPanel::setupDisplay).
static constexpr uint32_t LVGL_HEAP_BYTES             = 96 * 1024;

} // namespace waveshare