#ifndef __CONFIG_H
#define __CONFIG_H

#include <FS.h>
#include <LittleFS.h>
#include <Arduino.h>

#define CONFIG_FILE               "/config"
#define MAX_TIMERS                3
#define DEFAULT_USE_WIFI          false
#define DEFAULT_USE_SCREENSAVER   false
#define DEFAULT_USE_AP            true
#define DEFAULT_SCREENSAVER_TIME  5*60      // in seconds
#define DEFAULT_HOSTNAME          "Timer"
#define DEFAULT_PASSWORD          "SnowC4mp"
#define DEFAULT_BRIGHTNESS        127
#define DEFAULT_TIMER_COUNT       2
#define DEFAULT_TIMERS            { { 45, 15, 10, 5 }, { 20, 10, 5, 2 }, { 10, 5, 2, 1 } }

struct timer_settings_t {
  uint8_t duration;   // in minutes
  uint8_t warn1;      // remaining time in minutes
  uint8_t warn2;      // remaining time in minutes
  uint8_t warn3;      // remaining time in minutes
};

struct config_t {
  bool use_wifi = DEFAULT_USE_WIFI;
  bool use_screensaver = DEFAULT_USE_SCREENSAVER;
  bool use_ap = DEFAULT_USE_AP;
  char hostname[32] = DEFAULT_HOSTNAME;
  char password[32] = DEFAULT_PASSWORD;
  uint32_t screensaver_time = DEFAULT_SCREENSAVER_TIME;
  uint8_t brightness = DEFAULT_BRIGHTNESS;
  uint8_t timer_count = DEFAULT_TIMER_COUNT;
  timer_settings_t timers[MAX_TIMERS] = DEFAULT_TIMERS;
};

extern config_t config;

void saveConfig();
void loadConfig();
void resetConfig();

#endif // __CONFIG_H
