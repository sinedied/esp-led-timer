#include "config.h"

config_t config;

void saveConfig() {
  File configFile = LittleFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
  }

  configFile.write((uint8_t*)&config, sizeof(config));
  configFile.close();
  Serial.println("Config saved");
}

void loadConfig() {
  // Clean FS, if needed
  // LittleFS.format();

  Serial.println("Mounting FS...");

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount FS");
    return;
  }

  Serial.println("Mounted FS");

  if (LittleFS.exists(CONFIG_FILE)) {
    Serial.println("Reading config file");
    File configFile = LittleFS.open(CONFIG_FILE, "r");

    if (configFile) {
      size_t size = configFile.size();

      if (size != sizeof(config)) {
        Serial.println("Config file size mismatch");
        return;
      }

      configFile.read((u_int8_t*)&config, size);
      Serial.println("Config loaded");
    }
    configFile.close();
  }
  // If failed to open config file, stick to defaults
}
