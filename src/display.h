#ifndef __DISPLAY_H
#define __DISPLAY_H

// This is how many color levels the display shows - the more the slower the update
#define PxMATRIX_COLOR_DEPTH 3

// Defines the buffer height / the maximum height of the matrix
#define PxMATRIX_MAX_HEIGHT 32

// Defines the buffer width / the maximum width of the matrix
#define PxMATRIX_MAX_WIDTH 64

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

PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

#ifdef ESP32
void IRAM_ATTR display_updater() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(PxMATRIX_DEFAULT_SHOWTIME);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#elif ESP8266
// ISR for display refresh
void display_updater() {
  display.display(PxMATRIX_DEFAULT_SHOWTIME);
}
#endif

void display_update_enable(bool enable) {
#ifdef ESP32
  if (enable) {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &display_updater, true);
    timerAlarmWrite(timer, 4000, true);
    timerAlarmEnable(timer);
  } else {
    timerDetachInterrupt(timer);
    timerAlarmDisable(timer);
  }
#elif ESP8266
  if (enable)
    display_ticker.attach(0.004, display_updater);
  else
    display_ticker.detach();
#endif
}

#endif // __DISPLAY_H