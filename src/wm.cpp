#include "wm.h"

void ExtendedWiFiManager::startAPMode(char const *apName, char const *apPassword) {
  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("Starting manual AP mode"));
  #endif

  _begin();

  _apName     = apName;
  _apPassword = apPassword;

  if (_apName == "")
    _apName = getDefaultAPName();

  if (!validApPassword())
    return;

  startAP();
  WiFiSetCountry();
  setupDNSD();

  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("AP mode started"));
  #endif
}

void ExtendedWiFiManager::startServer() {
  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("Starting web server"));
  #endif

  if (_server_running) return;

  _server_running = true;

  server.reset(new WM_WebServer(_httpPort));
  server->on(WM_G(R_index), std::bind(&ExtendedWiFiManager::handleIndex, this));
  server->on(WM_G(R_controls), std::bind(&ExtendedWiFiManager::handleControls, this));
  server->on(WM_G(R_settings), std::bind(&ExtendedWiFiManager::handleSettings, this));
  server->on(WM_G(R_state), std::bind(&ExtendedWiFiManager::handleState, this));
  server->onNotFound (std::bind(&ExtendedWiFiManager::handleNotFound, this));
  server->begin();

  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("Web server started"));
  #endif
}

void ExtendedWiFiManager::stopServer() {
  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("Stopping web server"));
  #endif

  if (!_server_running) return;

  dnsServer->processNextRequest();
  dnsServer->stop();
  dnsServer.reset();

  server->handleClient();
  server->stop();
  server.reset();

  WiFi.softAPdisconnect(false);  
  delay(1000);
  WiFi_Mode(_usermode);

  if (WiFi.status() == WL_IDLE_STATUS){
    WiFi.reconnect();
    #ifdef WM_DEBUG_LEVEL
    DEBUG_WM(F("WiFi Reconnect, was idle"));
    #endif
  }

  _server_running = false;
  _end();
}

void ExtendedWiFiManager::processServer() {
  if (!_server_running) return;

  #if defined(WM_MDNS) && defined(ESP8266)
  MDNS.update();
  #endif

  dnsServer->processNextRequest();
  server->handleClient();
}

void ExtendedWiFiManager::handleIndex() {
  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("<- HTTP Index"));
  #endif

  if (captivePortal()) return; // If captive portal redirect instead of displaying the page
  handleRequest();

  String page = "index.html";
  HTTPSend(page);
}

void ExtendedWiFiManager::handleControls() {
  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("<- HTTP Controls"));
  #endif

  String page = "controls.html";
  HTTPSend(page);
}

void ExtendedWiFiManager::handleSettings() {
  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("<- HTTP Settings"));
  #endif

  String page = "settings.html";
  HTTPSend(page);
}

void ExtendedWiFiManager::handleState() {
  #ifdef WM_DEBUG_LEVEL
  DEBUG_WM(F("<- HTTP state"));
  DEBUG_WM(F("Method:"),server->method() == HTTP_GET  ? (String)FPSTR(S_GET) : (String)FPSTR(S_POST));
  #endif

  String page = "state";
  HTTPSend(page);

  // _ssid = server->arg(F("s")).c_str();
  // _pass = server->arg(F("p")).c_str();

  // if(_ssid == "" && _pass != ""){
  //   _ssid = WiFi_SSID(true); // password change, placeholder ssid, @todo compare pass to old?, confirm ssid is clean
  //   #ifdef WM_DEBUG_LEVEL
  //   DEBUG_WM(WM_DEBUG_VERBOSE,F("Detected WiFi password change"));
  //   #endif    
  // }

  // #ifdef WM_DEBUG_LEVEL
  // String requestinfo = "SERVER_REQUEST\n----------------\n";
  // requestinfo += "URI: ";
  // requestinfo += server->uri();
  // requestinfo += "\nMethod: ";
  // requestinfo += (server->method() == HTTP_GET) ? "GET" : "POST";
  // requestinfo += "\nArguments: ";
  // requestinfo += server->args();
  // requestinfo += "\n";
  // for (uint8_t i = 0; i < server->args(); i++) {
  //   requestinfo += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  // }

  // DEBUG_WM(WM_DEBUG_MAX,requestinfo);
  // #endif

  // // set static ips from server args
  // if (server->arg(FPSTR(S_ip)) != "") {
  //   //_sta_static_ip.fromString(server->arg(FPSTR(S_ip));
  //   String ip = server->arg(FPSTR(S_ip));
  //   optionalIPFromString(&_sta_static_ip, ip.c_str());
  //   #ifdef WM_DEBUG_LEVEL
  //   DEBUG_WM(WM_DEBUG_DEV,F("static ip:"),ip);
  //   #endif
  // }
  // if (server->arg(FPSTR(S_gw)) != "") {
  //   String gw = server->arg(FPSTR(S_gw));
  //   optionalIPFromString(&_sta_static_gw, gw.c_str());
  //   #ifdef WM_DEBUG_LEVEL
  //   DEBUG_WM(WM_DEBUG_DEV,F("static gateway:"),gw);
  //   #endif
  // }
  // if (server->arg(FPSTR(S_sn)) != "") {
  //   String sn = server->arg(FPSTR(S_sn));
  //   optionalIPFromString(&_sta_static_sn, sn.c_str());
  //   #ifdef WM_DEBUG_LEVEL
  //   DEBUG_WM(WM_DEBUG_DEV,F("static netmask:"),sn);
  //   #endif
  // }
  // if (server->arg(FPSTR(S_dns)) != "") {
  //   String dns = server->arg(FPSTR(S_dns));
  //   optionalIPFromString(&_sta_static_dns, dns.c_str());
  //   #ifdef WM_DEBUG_LEVEL
  //   DEBUG_WM(WM_DEBUG_DEV,F("static DNS:"),dns);
  //   #endif
  // }

  // String page;

  // if(_ssid == ""){
  //   page = getHTTPHead(FPSTR(S_titlewifisettings)); // @token titleparamsaved
  //   page += FPSTR(HTTP_PARAMSAVED);
  // }
  // else {
  //   page = getHTTPHead(FPSTR(S_titlewifisaved)); // @token titlewifisaved
  //   page += FPSTR(HTTP_SAVED);
  // }

  // if(_showBack) page += FPSTR(HTTP_BACKBTN);
  // page += FPSTR(HTTP_END);

  // server->sendHeader(FPSTR(HTTP_HEAD_CORS), FPSTR(HTTP_HEAD_CORS_ALLOW_ALL));
  // HTTPSend(page);
}

void ExtendedWiFiManager::checkForConnect(std::function<void(boolean)> callback) {
  if (!check_for_connect) {
    return;
  }

  uint8_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    _lastconxresult = WL_CONNECTED;
    check_for_connect = false;
    callback(true);
    return;
  } else if (status == WL_CONNECT_FAILED) {
    check_for_connect = false;
    callback(false);
    return;
  }
}
