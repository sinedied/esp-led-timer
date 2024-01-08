#ifndef __WM_H
#define __WM_H

#include <WiFiManager.h>

const char R_index[] PROGMEM = "/";
const char R_controls[] PROGMEM = "/controls.html";
const char R_settings[] PROGMEM = "/settings.html";
const char R_state[] PROGMEM = "/state";

class ExtendedWiFiManager : public WiFiManager {
  public:
    boolean check_for_connect = false;

    void startAPMode(char const* apName, char const* apPassword);
    void startServer();
    void stopServer();
    void processServer();
    void checkForConnect(std::function<void(boolean)> callback);

  private:
    boolean _server_running = false;

    void handleIndex();
    void handleControls();
    void handleSettings();
    void handleState();
};

#endif // __WM_H