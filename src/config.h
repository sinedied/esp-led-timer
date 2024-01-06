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

extern config_t config;

void saveConfig();
void loadConfig();

#endif // __CONFIG_H
