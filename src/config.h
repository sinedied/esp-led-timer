#ifndef __CONFIG_H
#define __CONFIG_H

#include <FS.h>
#include <LittleFS.h>
#include <Arduino.h>

#define CONFIG_FILE           "/config"
#define DEFAULT_USE_WIFI      false
#define DEFAULT_HOSTNAME      "Timer"
#define DEFAULT_BRIGHTNESS    127
#define DEFAULT_TIMER_COUNT   2
#define DEFAULT_TIMERS        { { 45, 15, 10, 5 }, { 20, 10, 5, 2 }, { 10, 5, 2, 1 } }

struct timer_settings_t {
  uint8_t duration;   // in minutes
  uint8_t warn1;      // remaining time in minutes
  uint8_t warn2;      // remaining time in minutes
  uint8_t warn3;      // remaining time in minutes
};

struct config_t {
  bool use_wifi = DEFAULT_USE_WIFI;
  char hostname[32] = DEFAULT_HOSTNAME;
  uint8_t brightness = DEFAULT_BRIGHTNESS;
  uint8_t timer_count = DEFAULT_TIMER_COUNT;
  timer_settings_t timers[3] = DEFAULT_TIMERS;
};

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

void initConfig() {
  // Clean FS, if needed
  // LittleFS.format();

  Serial.println("Mounting FS...");
  if (LittleFS.begin()) {
    Serial.println("Mounted FS");
    loadConfig();
  } else {
    Serial.println("Failed to mount FS");
  }
}

#endif // __CONFIG_H
