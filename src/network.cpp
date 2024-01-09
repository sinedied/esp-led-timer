#include "network.h"

AsyncWebServer server(80);
DNSServer dns_server;
boolean server_started = false;

void initWifi() {
  Serial.println("Init wifi");

  // Setup WiFi
  WiFi.persistent(false);
  WiFi.setAutoConnect(true);
  dns_server.setErrorReplyCode(DNSReplyCode::NoError);

  server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello from ESP");
  });

  server.on("/state", HTTP_GET, [&](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello from ESP");
  });

  server.on("/state", HTTP_POST, [&](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello from ESP");
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  if (config.use_wifi) {
    enableWifi(true);
  } else {
    Serial.println("Wifi disabled");
  }
}

void enableWifi(boolean enable) {
  Serial.printf("Enable wifi: %d\n", enable);
  config.use_wifi = enable;
  saveConfig();

  if (enable) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.hostname, config.password);
    Serial.println("IPAddress: " + WiFi.softAPIP().toString());
    dns_server.start(53, "*", WiFi.softAPIP());

    startWifiServer();
  } else {
    stopWifiServer();

    Serial.println("Disconnecting wifi");
    dns_server.stop();
    WiFi.softAPdisconnect(true);
  }
}

void resetWifi() {
  Serial.println("Resetting wifi settings");
  // No wifi settings for now
}

IPAddress getIP() {
  if (config.use_ap) {
    return WiFi.softAPIP();
  }
  return WiFi.localIP();
}

void startWifiServer() {
  Serial.println("Starting wifi server...");
  server.begin();
  server_started = true;
}

void stopWifiServer() {
  Serial.println("Stopping wifi server...");
  server.end();
  server_started = false;
}

void processServer() {
  if (!server_started) {
    return;
  }

  dns_server.processNextRequest();
}
