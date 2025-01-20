#include "network.h"

extern app_state_t state;

AsyncWebServer server(80);
DNSServer dns_server;
bool server_started = false;
control_callbacks_t callbacks;

void initWifi(control_callbacks_t& controls) {
  Serial.println("Init wifi");
  callbacks = controls;

  // Setup WiFi
  WiFi.persistent(false);
  WiFi.setAutoConnect(true);

  dns_server.setTTL(3600);
  dns_server.setErrorReplyCode(DNSReplyCode::NoError);

  server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request) {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", INDEX_HTML, INDEX_HTML_SIZE);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/state", HTTP_GET, [&](AsyncWebServerRequest *request) {
    String json = "{\"mode\":" + String(state.cur_mode);
    json += ",\"time\":" + String(state.cur_time);
    json += ",\"timer_started\":" + String(state.timer_started);
    json += ",\"brightness\":" + String(state.brightness) + "}";
    request->send(200, "application/json", json);
  });

  server.on("/state", HTTP_POST, [&](AsyncWebServerRequest *request) {
    // Data comes from the client as application/x-www-form-urlencoded (FormData)
    int8_t mode;
    if (getInt8(request, "mode", mode)) {
      if (mode != state.cur_mode) {
        DEBUG_NETWORK_PRINTF("/state set mode: %d\n", mode);
        callbacks.setMode(mode);
      }
    }

    uint8_t brightness;
    if (getUint8(request, "brightness", brightness)) {
      if (brightness != state.brightness) {
        DEBUG_NETWORK_PRINTF("/state set brightness: %u\n", brightness);
        callbacks.setBrightness(brightness);
      }
    }

    if (request->hasParam("timer_start", true)) {
      DEBUG_NETWORK_PRINTLN("/state start timer");
      callbacks.startTimer();
      callbacks.resetScreensaverTimer();
    }

    if (request->hasParam("timer_stop", true)) {
      DEBUG_NETWORK_PRINTLN("/state stop timer");
      callbacks.stopTimer();
      callbacks.resetScreensaverTimer();
    }

    if (request->hasParam("timer_reset", true)) {
      DEBUG_NETWORK_PRINTLN("/state reset timer");
      callbacks.resetTimer();
      callbacks.resetScreensaverTimer();
    }

    request->send(204);
  });

  server.on("/config", HTTP_GET, [&](AsyncWebServerRequest *request) {
    String json = "{\"use_wifi\":" + String(config.use_wifi);
    json += ",\"use_screensaver\":" + String(config.use_screensaver);
    json += ",\"use_ap\":" + String(config.use_ap);
    json += ",\"hostname\":\"" + String(config.hostname) + "\"";
    json += ",\"password\":\"" + String(config.password) + "\"";
    json += ",\"screensaver_time\":" + String(config.screensaver_time);
    json += ",\"brightness\":" + String(config.brightness);
    json += ",\"timer_count\":" + String(config.timer_count);
    json += ",\"timers\":[";

    for (int i = 0; i < MAX_TIMERS; i++) {
      json += "{\"duration\":" + String(config.timers[i].duration);
      json += ",\"warn1\":" + String(config.timers[i].warn1);
      json += ",\"warn2\":" + String(config.timers[i].warn2);
      json += ",\"warn3\":" + String(config.timers[i].warn3) + "}";

      if (i < MAX_TIMERS - 1) {
        json += ",";
      }
    }

    json += "]}";
    request->send(200, "application/json", json);
  });

  server.on("/config", HTTP_POST, [&](AsyncWebServerRequest *request) {
    // Data comes from the client as application/x-www-form-urlencoded (FormData)
    bool valid = getBool(request, "use_wifi", config.use_wifi)
      && getBool(request, "use_screensaver", config.use_screensaver)
      && getBool(request, "use_ap", config.use_ap)
      && getString(request, "hostname", config.hostname, sizeof(config.hostname))
      && getString(request, "password", config.password, sizeof(config.password))
      && getUint32(request, "screensaver_time", config.screensaver_time)
      && getUint8(request, "brightness", config.brightness)
      && getUint8(request, "timer_count", config.timer_count);

    for (int i = 0; i < MAX_TIMERS; i++) {
      String timer = "timer" + String(i);
      valid &= getUint8(request, (timer + "_duration").c_str(), config.timers[i].duration)
        && getUint8(request, (timer + "_warn1").c_str(), config.timers[i].warn1)
        && getUint8(request, (timer + "_warn2").c_str(), config.timers[i].warn2)
        && getUint8(request, (timer + "_warn3").c_str(), config.timers[i].warn3);
    }

    if (!valid) {
      // Error message has already been sent by getXxx() functions
      return;
    }

    // Request a config save and reboot (will be done in the main loop)
    callbacks.saveConfigAndReboot();
    request->send(204);
  });

  server.on("/favicon.ico", [&](AsyncWebServerRequest* request) {
    request->send(404);
  });

  server.onNotFound([&](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", INDEX_HTML, INDEX_HTML_SIZE);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  if (config.use_wifi) {
    enableWifi(true);
  } else {
    Serial.println("Wifi disabled");
  }
}

void enableWifi(bool enable) {
  Serial.printf("Enable wifi: %d\n", enable);

  if (enable != config.use_wifi) {
    config.use_wifi = enable;
    saveConfig();
  }

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
  if (!MDNS.begin("timer")) {
    Serial.println("Error setting up MDNS responder");
  }

  Serial.println("Starting wifi server...");
  server.begin();
  server_started = true;

  MDNS.addService("http", "tcp", 80);
}

void stopWifiServer() {
  MDNS.end();

  Serial.println("Stopping wifi server...");
  server.end();
  server_started = false;
}

void processServer() {
  if (!server_started) {
    return;
  }

  dns_server.processNextRequest();

#if defined(ESP8266)
  MDNS.update();
#endif
}

bool getBool(AsyncWebServerRequest* request, const char* field, bool& value) {
  if (!request->hasParam(field, true)) {
    return false;
  }

  String str = request->getParam(field, true)->value();
  DEBUG_NETWORK_PRINTF("%s: %s\n", field, str.c_str());
  value = str == "true";
  return true;
}

bool getInt8(AsyncWebServerRequest* request, const char* field, int8_t& value) {
  if (!request->hasParam(field, true)) {
    return false;
  }

  String str = request->getParam(field, true)->value();
  DEBUG_NETWORK_PRINTF("%s: %s\n", field, str.c_str());
  int32_t new_value;

  if (!sscanf(str.c_str(), "%d", &new_value)) {
    request->send(400, "text/plain", "Invalid value for " + String(field));
    return false;
  }

  value = int8_t(new_value);
  return true;
}

bool getUint8(AsyncWebServerRequest* request, const char* field, uint8_t& value) {
  if (!request->hasParam(field, true)) {
    return false;
  }

  String str = request->getParam(field, true)->value();
  DEBUG_NETWORK_PRINTF("%s: %s\n", field, str.c_str());
  uint32_t new_value;

  if (!sscanf(str.c_str(), "%u", &new_value)) {
    request->send(400, "text/plain", "Invalid value for " + String(field));
    return false;
  }

  value = uint8_t(new_value);
  return true;
}

bool getUint32(AsyncWebServerRequest* request, const char* field, uint32_t& value) {
  if (!request->hasParam(field, true)) {
    return false;
  }

  String str = request->getParam(field, true)->value();
  DEBUG_NETWORK_PRINTF("%s: %s\n", field, str.c_str());
  uint32_t new_value;

  if (!sscanf(str.c_str(), "%u", &new_value)) {
    request->send(400, "text/plain", "Invalid value for " + String(field));
    return false;
  }

  value = new_value;
  return true;
}

bool getString(AsyncWebServerRequest* request, const char* field, char* value, size_t max_len) {
  if (!request->hasParam(field, true)) {
    return false;
  }

  String str = request->getParam(field, true)->value();
  DEBUG_NETWORK_PRINTF("%s: %s\n", field, str.c_str());

  if (str.length() >= max_len) {
    request->send(400, "text/plain", "Invalid value for " + String(field));
    return false;
  }

  str.toCharArray(value, max_len);
  return true;
}
