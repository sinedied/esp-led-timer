#ifndef __NETWORK_H
#define __NETWORK_H

#define HSTNM "ssw32-littlefs"
#define MYFS LITTLEFS

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#ifdef ESP32
  // #include <WiFi.h>
  // #include <IPv6Address.h>
  // #include <WiFiClient.h>
  // #include <WiFiServer.h>
  // #include <WiFiAP.h>
  // #include <ESPmDNS.h>
#elif ESP8266
  // #include <ESP8266WiFi.h>
  // #include <ESP8266mDNS.h>
#endif

#include "wm.h"
#include "config.h"

ExtendedWiFiManager wifiManager;

void startWifiServer() {
  Serial.println("Starting wifi server...");
  wifiManager.startServer();
}

void processServer() {
  wifiManager.processServer();
}

void checkForConnect(std::function<void(boolean)> callback) {
  wifiManager.checkForConnect(callback);
}

void initAPMode() {
  Serial.println("Starting AP mode...");
  wifiManager.startAPMode(config.hostname, config.password);
}

void initWifi(bool enableDebug) {
  wifiManager.setDebugOutput(enableDebug);

  if (!config.use_wifi) {
    Serial.println("Wifi disabled");
    return;
  }

  if (!wifiManager.getWiFiIsSaved() || config.use_ap) {
    Serial.println("No wifi credentials saved or AP mode enabled");
    initAPMode();
  } else {
    wifiManager.setRestorePersistent(true);
    wifiManager.setConnectTimeout(1);
    wifiManager.setEnableConfigPortal(false);
    wifiManager.check_for_connect = true;
    wifiManager.autoConnect(config.hostname, config.password);

    // if (wifiManager.autoConnect(config.hostname, config.password)) {
    //   Serial.println("Connected to wifi");
    //   Serial.println(WiFi.localIP());
    // } else {
    //   Serial.println("Failed to connect to wifi");
    //   initAPMode();
    // }

    Serial.println("Wifi init done");
  }

  // startWifiServer();
}

IPAddress getIP() {
  return WiFi.localIP();
}

void setupWifi() {
  wifiManager.stopServer();

  // const char* menu[] = { "wifi", "setup", "sep", "exit" };
  // wifiManager.setMenu(menu, 4);

  wifiManager.setTitle("Wifi configuration");
  wifiManager.setConfigPortalTimeout(0);

  const char* config_use_wifi = config.use_wifi ? "1" : "0";
  WiFiManagerParameter use_wifi("use_wifi", "Enable wifi", config_use_wifi, 1);
  WiFiManagerParameter hostname("hostname", "Hostname", config.hostname, 32);
  WiFiManagerParameter password("password", "Password", config.password, 32);
  wifiManager.addParameter(&use_wifi);
  wifiManager.addParameter(&hostname);
  wifiManager.addParameter(&password);

  wifiManager.setSaveConfigCallback([&use_wifi, &hostname, &password]() {
    config.use_wifi = use_wifi.getValue()[0] != '0';
    strncpy(config.hostname, hostname.getValue(), 32);
    strncpy(config.password, password.getValue(), 32);
    Serial.printf("New hostname: %s\n", config.hostname);
    saveConfig();
  });

  wifiManager.setConfigPortalBlocking(false);
  // wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  wifiManager.startConfigPortal(config.hostname, config.password);
  // delay(1000);
  // ESP.restart();
  Serial.println("Web portal started");
}

void processSetup() {
  wifiManager.process();
}

void resetWifi() {
  Serial.println("Resetting wifi settings");
  wifiManager.resetSettings();
}

#endif // __NETWORK_H
