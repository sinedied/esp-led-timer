#include <Arduino.h>
#include <OneButton.h>
#include "display.h"
#include "images.h"

#define DEBUG_FAST  1       // Accelerate time x100 for debug
#define WIDTH       64      // Matrix width
#define HEIGHT      32      // Matrix height
#define TIMER_COUNT 2       // Number of different timers enabled (1-3)
#define STOP_AT     -10*60  // Stop timer at this time (in seconds)

#ifdef ESP32
  #define P_BUTTON    3     // Pin for the hardware button
#endif
#ifdef ESP8266
  #define P_BUTTON    D9    // Pin for the hardware button
#endif

enum app_mode_t {
  MODE_INFO = -1,
  MODE_LOGO = 0,
  MODE_TIMER_1,
  MODE_TIMER_2,
  MODE_TIMER_3
};

struct timer_settings {
  uint8_t duration;   // in minutes
  uint8_t warn1;      // remaining time in minutes
  uint8_t warn2;      // remaining time in minutes
  uint8_t warn3;      // remaining time in minutes
};

// TODO: for ESP32 use hardware timer
Ticker time_ticker;

// TODO: allow update from web interface
timer_settings timers[] = {
  { 45, 15, 10, 5 },
  { 20, 10, 5, 2 },
  { 10, 5, 2, 1 }
};

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

uint16_t progress_colors[] = {
  COLOR_GREEN,
  COLOR_YELLOW,
  COLOR_ORANGE,
  COLOR_RED,
  COLOR_MAGENTA
};

OneButton button = OneButton(P_BUTTON);
bool timer_started = false;
bool need_update = true;
uint8_t skip_update = 0;
int8_t cur_mode = MODE_INFO;
int cur_time = 0; // in seconds
uint8_t progress_bar_warn[3];

void computeProgressBarWarnZones() {
  timer_settings timer = timers[cur_mode - 1];
  progress_bar_warn[0] = (timer.duration - timer.warn1) * WIDTH / timer.duration;
  progress_bar_warn[1] = (timer.duration - timer.warn2) * WIDTH / timer.duration;
  progress_bar_warn[2] = (timer.duration - timer.warn3) * WIDTH / timer.duration;
}

static void resetTimer() {
  switch (cur_mode) {
    case MODE_INFO:
    case MODE_LOGO:
      break;
    default:
      cur_time = timers[cur_mode - 1].duration * 60;
      computeProgressBarWarnZones();
  }

  timer_started = false;
  time_ticker.detach();
}

static void nextMode() {
  cur_mode = (cur_mode + 1) % (TIMER_COUNT + 1);
  resetTimer();
  need_update = true;
  skip_update = 0;
  Serial.printf("Set mode: %d\n", cur_mode);
}

static void onPush() {
  timer_started = !timer_started;

  if (cur_mode == MODE_INFO) {
    nextMode();
    nextMode();
  } else if (cur_mode != MODE_LOGO && timer_started) {
    time_ticker.attach(DEBUG_FAST ? 0.01 : 1.0, []() -> void {
      if (--cur_time <= STOP_AT) {
        resetTimer();
      }
    });
  } else {
    time_ticker.detach();
  }
}

void drawBitmap(uint8_t x, uint8_t y, const uint16_t* bitmap, uint8_t w, uint8_t h) {
 int pixel = 0;
 for (int yy = 0; yy < h; yy++) {
   for (int xx = 0; xx < w; xx++) {
     display.drawPixel(xx + x , yy + y, bitmap[pixel]);
     pixel++;
   }
 }
}

void drawProgressbar() {
  timer_settings timer = timers[cur_mode - 1];

  int curr = WIDTH - cur_time * WIDTH / timer.duration / 60;
  int warn1 = (timer.duration - timer.warn1) * WIDTH / timer.duration;
  int warn2 = (timer.duration - timer.warn2) * WIDTH / timer.duration;
  int warn3 = (timer.duration - timer.warn3) * WIDTH / timer.duration;

  if (cur_time > 0) {
    display.writeFillRect(0, 28, warn1, 4, progress_colors[0]);
    display.writeFillRect(warn1, 28, warn2 - warn1, 4, progress_colors[1]);
    display.writeFillRect(warn2, 28, warn3 - warn2, 4, progress_colors[2]);
    display.writeFillRect(warn3, 28, WIDTH - warn3, 4, progress_colors[3]);
    display.writeFillRect(0, 28, curr, 4, COLOR_BLACK);

    // Draw white triangle
    display.writeFastHLine(curr - 1, 26, 3, COLOR_WHITE);
    display.writePixel(curr, 27, COLOR_WHITE);
  } else if (cur_time % 2 == 0) {
    // If time negative, flash message every second
    display.writeFillRect(0, 25, 4, 4, progress_colors[4]);
    display.writeFillRect(60, 25, 4, 4, progress_colors[4]);

    display.setTextColor(progress_colors[4]);
    display.setTextSize(1);
    display.setCursor(5, 23);
    display.print("TIME OVER");
  }
}

void showTimer() {
  if (skip_update > 0) {
    skip_update--;
    return;
  }

  // Update every 33 ticks (~30fps)
  skip_update = 33; 
  display.fillScreen(COLOR_BLACK);

  timer_settings timer = timers[cur_mode - 1];
  int cur_min = cur_time / 60;
  uint16_t cur_color = progress_colors[0];
  
  if (cur_time <= 0) {
    cur_color = progress_colors[4];
  } else if (cur_min <= timer.warn3) {
    cur_color = progress_colors[3];
  } else if (cur_min <= timer.warn2) {
    cur_color = progress_colors[2];
  } else if (cur_min <= timer.warn1) {
    cur_color = progress_colors[1];
  }

  display.setTextColor(cur_color);
  display.setTextSize(2);
  display.setCursor(27, 6);
  display.print(":");
  display.setCursor(6, 6);
  display.printf("%02d", abs(cur_min));
  display.setCursor(36, 6);
  display.printf("%02d", abs(cur_time % 60));

  if (cur_time < 0) {
    display.writeFillRect(0, 12, 4, 2, cur_color);
  }

  drawProgressbar();
  display.showBuffer();
}

void showLogo() {
  if (need_update) {
    display.fillScreen(COLOR_BLACK);
    drawBitmap(0, 0, bmp_snowcamp_64x32, 64, 32);
    display.showBuffer();
  }
  need_update = false;
}

void showInfo() {
  if (need_update) {
    display.fillScreen(COLOR_BLACK);
    drawBitmap(0, 0, bmp_manual_64x32, 64, 32);
    display.showBuffer();
  }
  need_update = false;
}

void setup() {
  Serial.begin(9600);

  // 64x32 = 1/16 scan mode
  display.begin(16);
  display.clearDisplay();
  display_update_enable(true);

  // Set the brightness of the panels (default is 255)
  display.setBrightness(255);

  pinMode(P_BUTTON, INPUT);
  button.attachClick(onPush);
  button.attachDoubleClick(nextMode);
  button.setPressTicks(1000);
  button.attachLongPressStart(resetTimer);

  showLogo();
  delay(3000);
  need_update = true;
}

void loop() {
  button.tick();

  if (cur_mode == MODE_INFO) {
    showInfo();
  } else if (cur_mode == MODE_LOGO) {
    showLogo();
  } else {
    showTimer();
  }
}
