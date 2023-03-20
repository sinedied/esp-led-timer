#include <Arduino.h>
#include <OneButton.h>
#include "display.h"
#include "images.h"

// Number of different timers enabled (1-3)
#define TIMER_COUNT 2

// Pin for the hardware button
#define P_BUTTON    D9

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

// TODO: adapt for ESP32
Ticker time_ticker;
OneButton button = OneButton(P_BUTTON);
bool timer_started = false;

// TODO: replace by need update flag
bool has_updated = false;

// TODO: use int for time
struct hmstime_t {
  uint8_t h;
  uint8_t m;
  uint8_t s;
};

hmstime_t cur_time = { 0, 0, 0 };

// TODO: allow update from web interface
timer_settings timers[] = {
  { 45, 15, 10, 5 },
  { 20, 10, 5, 2 },
  { 10, 5, 3, 1 }
};

int8_t cur_mode = MODE_INFO;

static void updateTime() {
  if (cur_time.s == 0) {
    cur_time.s = 59;
    cur_time.m--;
    if (cur_time.m == 0) {
      cur_time.m = 59;
      cur_time.h--;
      if (cur_time.h == 0) {
        cur_time.h = 23;
        // TODO: end of timer
      }
    }
  }
  cur_time.s--;
}

static void resetTimer() {
  switch (cur_mode) {
    case MODE_INFO:
    case MODE_LOGO:
      break;
    default:
      cur_time.h = timers[cur_mode - 1].duration / 60;
      cur_time.m = timers[cur_mode - 1].duration % 60;
      cur_time.s = 0;
  }

  timer_started = false;
  time_ticker.detach();
}

static void nextMode() {
  cur_mode = cur_mode + 1 % (TIMER_COUNT + 1);
  resetTimer();
  has_updated = false;
}

static void onPush() {
  timer_started = !timer_started;

  if (cur_mode == MODE_INFO) {
    nextMode();
  } else if (cur_mode != MODE_LOGO && timer_started) {
    time_ticker.attach(1.0, updateTime);
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

void showText(String text, uint8_t colorR, uint8_t colorG, uint8_t colorB) {
  display.fillScreen(COLOR_BLACK);
  // display.setTextSize(1);
  // display.setTextColor(display.color565(colorR,colorG,colorB));
  display.setTextColor(COLOR_CYAN);
  display.setCursor(2,2);
  display.println(text);
}

void showTimer() {
  display.fillScreen(COLOR_BLACK);
  display.setTextSize(2);

  // TODO: check color/warnings
  uint16_t cur_color = COLOR_GREEN;
  display.setTextColor(cur_color);

  display.setCursor(28,8);
  display.print(":");
  display.setCursor(4,8);
  display.printf("%02d", cur_time.m);
  display.setCursor(39,8);
  display.printf("%02d", cur_time.s);

  // TODO: draw progress bar with white triangle pos

  display.writeFastHLine(0, 28, 64, cur_color);
  display.writeFastHLine(0, 29, 64, cur_color);
  display.writeFastHLine(0, 30, 64, cur_color);
  display.writeFastHLine(0, 31, 64, cur_color);

  display.showBuffer();
  delay(20);
}

void showLogo() {
  if (!has_updated) {
    display.fillScreen(COLOR_BLACK);
    drawBitmap(0, 0, bmp_snowcamp_64x32, 64, 32);
    display.showBuffer();
  }
  has_updated = true;
}

void showInfo() {
  if (!has_updated) {
    display.fillScreen(COLOR_BLACK);
    drawBitmap(0, 0, bmp_manual_64x32, 64, 32);
    display.showBuffer();
  }
  has_updated = true;
}

void setup() {
  Serial.begin(9600);

  // 64x32 = 1/16 scan mode
  display.begin(16);
  display.clearDisplay();
  display_update_enable(true);

  // Set the brightness of the panels (default is 255)
  //display.setBrightness(50);

  pinMode(P_BUTTON, INPUT);
  button.attachClick(onPush);
  button.attachDoubleClick(nextMode);
  button.setPressTicks(1000);
  button.attachLongPressStart(resetTimer);

  showLogo();
  delay(3000);
  has_updated = false;
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
