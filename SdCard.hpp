#pragma once

// =====================================================================
//  SdCard – SD-Karten-Wrapper (SPI + CH422G-CS)
//  ----------------------------------------------------------------------
//  Verwendet die gleiche Pinbelegung wie das boardport-03_SD_Test-Projekt:
//    MOSI=35, MISO=37, CLK=36, CS=34 (Chip Select direkt am GPIO,
//    im boardport zusätzlich über CH422G verstärkt).
//
//  Erfordert die Arduino-`SD`-Bibliothek. Sie wird per
//      lib_deps = SD
//  in platformio.ini eingebunden.
// =====================================================================

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "BoardConfig.hpp"
#include "I2CBus.hpp"
#include "ExpanderCH422G.hpp"

namespace waveshare {

class SdCard {
 public:
  // Mountet die SD-Karte. format_if_mount_failed entspricht dem
  // boardport-Default (CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED).
  static bool begin(bool format_if_mount_failed = false,
                    uint8_t max_files = SD_MAX_FILES_OPEN,
                    uint32_t alloc_unit = SD_ALLOCATION_UNIT_SIZE) {
    if (s_mounted) return true;

    if (!I2CBus::begin(Wire, PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_HZ)) {
      return false;
    }
    if (!ExpanderCH422G::begin()) {
      return false;
    }

    // CS-Pin konfigurieren (direkt am GPIO, im boardport zusätzlich
    // über CH422G verstärkt — wir verlassen uns auf die direkte GPIO).
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH);

    // SPI-Bus initialisieren. Arduino's SD.begin(SPI-Konfiguration)
    // nimmt CLK, MISO, MOSI aus SPISettings, daher initialisieren wir
    // SPI nur, falls noch nicht geschehen.
    static bool spi_initialised = false;
    if (!spi_initialised) {
      SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
      spi_initialised = true;
    }

    if (!SD.begin(PIN_SD_CS, SPI, SD_SPI_CLOCK_HZ, SD_MOUNT_POINT,
                  max_files, format_if_mount_failed)) {
      s_mounted = false;
      return false;
    }

    s_mounted = (SD.cardType() != CARD_NONE);
    return s_mounted;
  }

  static void end() {
    SD.end();
    s_mounted = false;
  }

  static bool isMounted() { return s_mounted; }
  static const char *mountPoint() { return SD_MOUNT_POINT; }

  // Datei-Helper
  static bool writeFile(const char *path, const char *data) {
    if (!s_mounted) return false;
    File f = SD.open(path, FILE_WRITE);
    if (!f) return false;
    size_t written = f.print(data);
    f.close();
    return written == strlen(data);
  }

  static String readFile(const char *path) {
    if (!s_mounted) return String();
    File f = SD.open(path, FILE_READ);
    if (!f) return String();
    String content = f.readString();
    f.close();
    return content;
  }

  static bool exists(const char *path) {
    if (!s_mounted) return false;
    File f = SD.open(path, FILE_READ);
    bool ok = static_cast<bool>(f);
    if (f) f.close();
    return ok;
  }

  // Test-Routine wie waveshare_sd_card_test() im boardport
  static bool runSelfTest(bool format_after = false) {
    if (!s_mounted) return false;

    String path = String(SD_MOUNT_POINT) + "/hello.txt";
    if (!writeFile(path.c_str(), "Hello from Waveshare driver!\n")) return false;
    String content = readFile(path.c_str());
    if (content.indexOf("Hello") < 0) return false;

    String renamed = String(SD_MOUNT_POINT) + "/foo.txt";
    if (!SD.rename(path.c_str(), renamed.c_str())) return false;

    String finalPath = String(SD_MOUNT_POINT) + "/nihao.txt";
    if (!SD.rename(renamed.c_str(), finalPath.c_str())) return false;
    SD.remove(finalPath.c_str());

    if (format_after) {
      File root = SD.open(SD_MOUNT_POINT);
      if (root && root.isDirectory()) {
        File entry = root.openNextFile();
        while (entry) {
          if (!entry.isDirectory()) {
            SD.remove(entry.path());
          }
          entry = root.openNextFile();
        }
        root.close();
      }
    }

    return true;
  }

 private:
  static inline bool s_mounted = false;
};

} // namespace waveshare