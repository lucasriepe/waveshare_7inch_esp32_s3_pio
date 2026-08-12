#pragma once

// =====================================================================
//  Waveshare ESP32-S3 Touch LCD 7 - Hardwarekonfiguration
//  ----------------------------------------------------------------------
//  Übernimmt die exakten Werte aus dem boardport/-Referenzprojekt
//  (08_lvgl_Porting, 03_SD_Test, 06_TWAItransmit, 07_TWAIreceive,
//   04_Sensor_AD, 02_RS485_Test/05_UART_Test, 01_I2C_Test).
//
//  Alle Werte sind constexpr, damit sie ohne Kconfig/SDKCONFIG
//  bereits zur Compile-Zeit bekannt sind. Der Arduino-Build
//  (platformio.ini) enthält bereits PSRAM + RGB-Element-Order-Flags.
// =====================================================================

#include <stdint.h>

namespace waveshare {

// --- Display-Geometrie ------------------------------------------------
static constexpr uint16_t LCD_WIDTH        = 800;
static constexpr uint16_t LCD_HEIGHT       = 480;
static constexpr uint32_t LCD_PIXEL_CLOCK_HZ = 16u * 1000u * 1000u; // 16 MHz
static constexpr uint8_t  LCD_BITS_PER_PIXEL = 16;

// RGB-Timing (entspricht EXAMPLE_RGB_HSYNC/VSYNC im boardport)
static constexpr uint32_t LCD_HSYNC_PULSE_WIDTH = 4;
static constexpr uint32_t LCD_HSYNC_BACK_PORCH  = 8;
static constexpr uint32_t LCD_HSYNC_FRONT_PORCH = 8;
static constexpr uint32_t LCD_VSYNC_PULSE_WIDTH = 4;
static constexpr uint32_t LCD_VSYNC_BACK_PORCH  = 8;
static constexpr uint32_t LCD_VSYNC_FRONT_PORCH = 8;
static constexpr uint32_t LCD_PCLK_ACTIVE_NEG   = 1;
static constexpr uint32_t LCD_SRAM_ALIGN        = 4;
static constexpr uint32_t LCD_PSRAM_ALIGN       = 64;
static constexpr uint32_t LCD_BB_LINES          = 10; // bounce buffer

// --- RGB-Daten-Pins ---------------------------------------------------
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

// --- I2C-Bus (shared) ------------------------------------------------
//  Aus 03_SD_Test, 06_TWAItransmit und 08_lvgl_Porting identisch:
//  SDA = 8, SCL = 9, Port 0, 400 kHz, CH422G-Expander @ 0x24/0x38
static constexpr int8_t  PIN_I2C_SDA         = 8;
static constexpr int8_t  PIN_I2C_SCL         = 9;
static constexpr uint32_t I2C_CLOCK_HZ      = 400 * 1000;
static constexpr uint8_t I2C_EXPANDER_CFG_ADDR = 0x24;
static constexpr uint8_t I2C_EXPANDER_DATA_ADDR = 0x38;

// CH422G Ausgabebyte-Konstanten (siehe waveshare_rgb_lcd_port.c)
static constexpr uint8_t CH422G_TOUCH_RST_PRE   = 0x2C;
static constexpr uint8_t CH422G_TOUCH_RST_ASSERT = 0x2E;
static constexpr uint8_t CH422G_BL_ON            = 0x1E;
static constexpr uint8_t CH422G_BL_OFF           = 0x1A;
static constexpr uint8_t CH422G_USB_SEL_HIGH     = 0x20; // TWAI <-> USB

// Aux-GPIO für Touch-Reset (synchronized mit GPIO4 im boardport)
static constexpr int8_t PIN_TOUCH_AUX = 4;

// --- Touch (GT911) ---------------------------------------------------
//  Adresse wird über esp_lcd_touch_helper hinterlegt
//  Standardmäßig 0x5D, alternativ 0x14 (wird per Helper gewählt)
static constexpr uint8_t  TOUCH_I2C_ADDR_PRIMARY   = 0x5D;
static constexpr uint8_t  TOUCH_I2C_ADDR_SECONDARY = 0x14;
static constexpr uint16_t TOUCH_X_MAX              = LCD_WIDTH;
static constexpr uint16_t TOUCH_Y_MAX              = LCD_HEIGHT;

// --- SD-Karte (SPI) --------------------------------------------------
static constexpr int8_t PIN_SD_MOSI = 35;
static constexpr int8_t PIN_SD_MISO = 37;
static constexpr int8_t PIN_SD_CLK  = 36;
static constexpr int8_t PIN_SD_CS   = 34;
static constexpr uint32_t SD_SPI_CLOCK_HZ = 20 * 1000 * 1000; // SDMMC_FREQ_DEFAULT
static constexpr uint32_t SD_MAX_TRANSFER_SZ = 4000;
static constexpr uint8_t  SD_MAX_FILES_OPEN  = 5;
static constexpr uint32_t SD_ALLOCATION_UNIT_SIZE = 16 * 1024;
static constexpr const char *SD_MOUNT_POINT = "/sdcard";

// --- TWAI / CAN ------------------------------------------------------
static constexpr int8_t PIN_TWAI_TX = 0;
static constexpr int8_t PIN_TWAI_RX = 2;
static constexpr uint32_t TWAI_BITRATE_50KBPS  = 50000;
static constexpr uint32_t TWAI_BITRATE_500KBPS = 500000;
static constexpr uint32_t TWAI_TRANSMIT_RATE_MS = 1000;

// --- ADC -------------------------------------------------------------
static constexpr int8_t  PIN_ADC_CHANNEL = 6;      // GPIO6 = ADC1_CH5
static constexpr uint32_t ADC_SAMPLE_PERIOD_MS = 1000;

// --- UART ------------------------------------------------------------
static constexpr uint32_t UART_BAUD = 115200;
static constexpr uint8_t  UART_PORT = 2;
static constexpr int8_t   UART_PIN_TX = 4;
static constexpr int8_t   UART_PIN_RX = 5;
static constexpr uint16_t UART_BUF_SIZE = 1024;
static constexpr uint16_t UART_TASK_STACK_SIZE = 2048;

// --- LVGL Defaults ---------------------------------------------------
static constexpr uint32_t LVGL_TICK_PERIOD_MS     = 2;
static constexpr uint32_t LVGL_TASK_MAX_DELAY_MS  = 500;
static constexpr uint32_t LVGL_TASK_MIN_DELAY_MS  = 10;
static constexpr uint32_t LVGL_TASK_STACK_SIZE    = 6 * 1024;
static constexpr uint8_t  LVGL_TASK_PRIORITY      = 2;
static constexpr int8_t   LVGL_TASK_CORE          = -1;
static constexpr uint16_t LVGL_BUFFER_LINES       = 100;

} // namespace waveshare
