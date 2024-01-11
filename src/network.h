#ifndef __NETWORK_H
#define __NETWORK_H

#define HSTNM "ssw32-littlefs"
#define MYFS LITTLEFS

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h> 

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <DNSServer.h>
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <DNSServer.h>
  #include <AsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#include "config.h"
#include "html.h"
#include "types.h"

extern AsyncWebServer server;
extern DNSServer dns_server;
extern bool server_started;

struct control_callbacks_t {
  std::function<void()> startTimer;
  std::function<void()> stopTimer;
  std::function<void()> resetTimer;
  std::function<void(uint8_t)> setBrightness;
  std::function<void(uint8_t)> setMode;
  std::function<void()> resetScreensaverTimer;
};

void initWifi(control_callbacks_t& callbacks);
void enableWifi(bool enable);
void resetWifi();
IPAddress getIP();

void startWifiServer();
void stopWifiServer();
void processServer();

#endif // __NETWORK_H
