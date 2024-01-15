#ifndef __NETWORK_H
#define __NETWORK_H

#define HSTNM "ssw32-littlefs"
#define MYFS LITTLEFS

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h> 

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#endif

#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "html.h"
#include "types.h"

// #define DEBUG_NETWORK  1

#if DEBUG_NETWORK
  #define DEBUG_NETWORK_PRINTF(...)   Serial.printf(__VA_ARGS__)
  #define DEBUG_NETWORK_PRINTLN(x)    Serial.println(x)
#else
  #define DEBUG_NETWORK_PRINTF(...)
  #define DEBUG_NETWORK_PRINTLN(x)
#endif

extern AsyncWebServer server;
extern DNSServer dns_server;
extern bool server_started;

struct control_callbacks_t {
  std::function<void()> startTimer;
  std::function<void()> stopTimer;
  std::function<void()> resetTimer;
  std::function<void(uint8_t)> setBrightness;
  std::function<void(int8_t)> setMode;
  std::function<void()> resetScreensaverTimer;
  std::function<void()> saveConfigAndReboot;
};

void initWifi(control_callbacks_t& callbacks);
void enableWifi(bool enable);
void resetWifi();
IPAddress getIP();

void startWifiServer();
void stopWifiServer();
void processServer();

// Helper functions for parsing form data
bool getBool(AsyncWebServerRequest* request, const char* field, bool& value);
bool getInt8(AsyncWebServerRequest* request, const char* field, int8_t& value);
bool getUint8(AsyncWebServerRequest* request, const char* field, uint8_t& value);
bool getUint32(AsyncWebServerRequest* request, const char* field, uint32_t& value);
bool getString(AsyncWebServerRequest* request, const char* field, char* value, size_t max_len);

#endif // __NETWORK_H
