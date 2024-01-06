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
#include "config.h"

#define CONNECT_TIMEOUT 10     // in seconds
#define CONFIG_TIMEOUT  10*60  // in seconds

WiFiManager wifiManager;

void startWifiServer() {
  Serial.println("Starting wifi server...");
  // TODO: implement
}

void initAPMode() {
  Serial.println("Starting AP mode...");
  // TODO: if no credentials are stored, start in AP mode
}

void initWifi(bool enableDebug) {
  wifiManager.setDebugOutput(enableDebug);

  if (!config.use_wifi) {
    Serial.println("Wifi disabled");
    return;
  }

  if (!wifiManager.getWiFiIsSaved()) {
    Serial.println("No wifi credentials saved");
    initAPMode();
  } else {
    wifiManager.setRestorePersistent(true);
    wifiManager.setConnectTimeout(CONNECT_TIMEOUT);
    wifiManager.setEnableConfigPortal(false);

    if (wifiManager.autoConnect(config.hostname, config.password)) {
      Serial.println("Connected to wifi");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("Failed to connect to wifi");
      initAPMode();
    }
  }

  startWifiServer();
}

void setupWifi() {
  // const char* menu[] = { "wifi", "setup", "sep", "exit" };
  // wifiManager.setMenu(menu, 4);

  wifiManager.setTitle("Wifi configuration");
  wifiManager.setConfigPortalTimeout(CONFIG_TIMEOUT);

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

  wifiManager.startConfigPortal(config.hostname, config.password);
  delay(1000);
  ESP.restart();
}

#endif // __WIFI_H
