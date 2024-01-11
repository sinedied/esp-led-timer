#ifndef __DISPLAY_H
#define __DISPLAY_H

#define DISPLAY_WIDTH   64      // Matrix width
#define DISPLAY_HEIGHT  32      // Matrix height

// This is how many color levels the display shows - the more the slower the update
#define PxMATRIX_COLOR_DEPTH 3

// Defines the buffer height / the maximum height of the matrix
#define PxMATRIX_MAX_HEIGHT DISPLAY_HEIGHT

// Defines the buffer width / the maximum width of the matrix
#define PxMATRIX_MAX_WIDTH DISPLAY_WIDTH

// Defines how long we display things by default
// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash (10-70 should be fine)
#define PxMATRIX_DEFAULT_SHOWTIME 40

// Defines the speed of the SPI bus (reducing this may help if you experience noisy images)
#define PxMATRIX_SPI_FREQUENCY 10000000

// Creates a second buffer for backround drawing (doubles the required RAM)
#define PxMATRIX_double_buffer true

#include <PxMatrix.h>
#include <Ticker.h>
#include "config.h"

// Pins for LED MATRIX
#ifdef ESP32

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_OE 16
hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#elif ESP8266

Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_OE 2

#endif

PxMATRIX display(DISPLAY_WIDTH, DISPLAY_HEIGHT, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// Some standard colors
uint16_t COLOR_RED = display.color565(255, 0, 0);
uint16_t COLOR_GREEN = display.color565(0, 255, 0);
uint16_t COLOR_BLUE = display.color565(0, 0, 255);
uint16_t COLOR_WHITE = display.color565(255, 255, 255);
uint16_t COLOR_YELLOW = display.color565(255, 255, 0);
uint16_t COLOR_ORANGE = display.color565(255, 79, 0);
uint16_t COLOR_CYAN = display.color565(0, 255, 255);
uint16_t COLOR_MAGENTA = display.color565(223, 0, 191);
uint16_t COLOR_BLACK = display.color565(0, 0, 0);

#ifdef ESP32
void IRAM_ATTR displayUpdater() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(PxMATRIX_DEFAULT_SHOWTIME);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#elif ESP8266
// ISR for display refresh
void displayUpdater() {
  display.display(PxMATRIX_DEFAULT_SHOWTIME);
}
#endif

void displayUpdateEnable(bool enable) {
#ifdef ESP32
  if (enable) {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &displayUpdater, true);
    timerAlarmWrite(timer, 4000, true);
    timerAlarmEnable(timer);
  } else {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
#elif ESP8266
  if (enable)
    // When using wifi, we need to update the display less often
    // otherwise the DHCP server can't respond to requests fast enough
    display_ticker.attach(config.use_wifi ? 0.008 : 0.004, displayUpdater);
  else
    display_ticker.detach();
#endif
}

void initDisplay(uint8_t brightness) {
  // 64x32 = 1/16 scan mode
  display.begin(16);
  display.clearDisplay();
  display.setBrightness(brightness);
  display.setTextWrap(false);
  displayUpdateEnable(true);
}

#endif // __DISPLAY_H