#ifndef __WIFI_H
#define __WIFI_H

#include <Arduino.h>

#ifdef ESP32
  #include <IPv6Address.h>
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WiFiServer.h>
  #include <WiFiAP.h>
#endif

#include <WiFiManager.h>

#define DEVICE_NAME     "Timer"
#define DEVICE_PASSWORD "SnowC4amp"

void wifiSetup() {
  WiFiManager wifiManager;
  wifiManager.autoConnect(DEVICE_NAME, DEVICE_PASSWORD);
}

#endif // __WIFI_H