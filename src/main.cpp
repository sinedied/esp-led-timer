#include <Arduino.h>
#include <OneButton.h>
#include "display.h"
#include "bitmaps.h"
#include "extra.h"
#include "config.h"
#include "wifi.h"

// #define DEBUG_SIM   1       // Simulate display on serial
#define DEBUG_LOGS  1       // Enable debug logs
#define DEBUG_FAST  0       // Accelerate time x100 for debug
#define STOP_AT     -9*60   // Stop overtime timer X seconds
#define CONFIG_HOLD 5       // Hold button for X seconds to enter config mode

#ifdef ESP32
  #define P_BUTTON    RX    // Pin for the hardware button
  #define SKIP_UPDATE 120   // Update every n cycles
#elif ESP8266
  #define P_BUTTON    D9    // Pin for the hardware button
  #define SKIP_UPDATE 60    // Update every n cycles
#endif

#ifdef DEBUG_SIM
  #define debugSim(...)   Serial.printf(__VA_ARGS__)
  #undef SKIP_UPDATE
  #ifdef ESP32
    #define SKIP_UPDATE 1200
  #elif ESP8266
    #define SKIP_UPDATE 600
  #endif
#else
  #define debugSim(...)
#endif

enum app_mode_t {
  MODE_UNDEF = -100,
  MODE_CONFIG = -2,
  MODE_INFO = -1,
  MODE_LOGO = 0,
  MODE_TIMER_N,
};

// TODO: use system_get_time for best precision
Ticker time_ticker;
Ticker screensaver_ticker;

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
uint16_t skip_update = 0;
int8_t cur_mode = MODE_INFO;
int8_t prev_mode = MODE_UNDEF;
int cur_time = 0; // in seconds
uint8_t progress_bar_warn[3];
uint8_t brightness;
bool fast_time = DEBUG_FAST;
uint8_t cycle = 0;
time_t press_start;

static void nextMode(int8_t mode);

time_t getTime() {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  return current_time.tv_sec;
}

void resetScreensaverTimer() {
  screensaver_ticker.detach();
  if (config.use_screensaver && cur_mode != MODE_LOGO && !timer_started) {
    screensaver_ticker.once(config.idle_time, []() -> void {
      prev_mode = cur_mode;
      nextMode(MODE_LOGO);
    });
  }
}

void computeProgressBarWarnZones() {
  timer_settings_t timer = config.timers[cur_mode - 1];
  progress_bar_warn[0] = (timer.duration - timer.warn1) * DISPLAY_WIDTH / timer.duration;
  progress_bar_warn[1] = (timer.duration - timer.warn2) * DISPLAY_WIDTH / timer.duration;
  progress_bar_warn[2] = (timer.duration - timer.warn3) * DISPLAY_WIDTH / timer.duration;
}

static void resetTimer() {
  if (cur_mode == MODE_TIMER_N) {
    cur_time = config.timers[cur_mode - 1].duration * 60;
    computeProgressBarWarnZones();
  }

  timer_started = false;
  time_ticker.detach();
  resetScreensaverTimer();
  press_start = getTime();
}

static void checkForConfigMode() {
  if (cur_mode != MODE_CONFIG) {
    time_t press_since = getTime() - press_start;
    if (press_since > CONFIG_HOLD) {
      nextMode(MODE_CONFIG);
    }
  }
}

static void nextMode(int8_t mode = MODE_UNDEF) {
  if (cur_mode == MODE_CONFIG) {
    ESP.restart();
  } else if (prev_mode != MODE_UNDEF && mode >= MODE_TIMER_N) {
    cur_mode = prev_mode;
    prev_mode = MODE_UNDEF;
  } else {
    cur_mode = mode == MODE_UNDEF ? (cur_mode + 1) % (config.timer_count + 1) : mode;
  }
  resetTimer();
  need_update = true;
  skip_update = 0;
  Serial.printf("Set mode: %d\n", cur_mode);
}

static void onPush() {
  if (cur_mode < MODE_TIMER_N) {
    nextMode(MODE_TIMER_N);
  } else {
    if (timer_started) {
      time_ticker.detach();
    } else {
      time_ticker.attach(fast_time ? 0.01 : 1.0, []() -> void {
        if (--cur_time <= STOP_AT) {
          resetTimer();
        }
      });
    }
    timer_started = !timer_started;
  }
  resetScreensaverTimer();
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
  timer_settings_t timer = config.timers[cur_mode - 1];

  int curr = DISPLAY_WIDTH - cur_time * DISPLAY_WIDTH / timer.duration / 60;
  int warn1 = (timer.duration - timer.warn1) * DISPLAY_WIDTH / timer.duration;
  int warn2 = (timer.duration - timer.warn2) * DISPLAY_WIDTH / timer.duration;
  int warn3 = (timer.duration - timer.warn3) * DISPLAY_WIDTH / timer.duration;

  if (cur_time > 0) {
    display.writeFillRect(0, 28, warn1, 4, progress_colors[0]);
    display.writeFillRect(warn1, 28, warn2 - warn1, 4, progress_colors[1]);
    display.writeFillRect(warn2, 28, warn3 - warn2, 4, progress_colors[2]);
    display.writeFillRect(warn3, 28, DISPLAY_WIDTH - warn3, 4, progress_colors[3]);
    display.writeFillRect(0, 28, curr, 4, COLOR_BLACK);

    // Draw white triangle
    display.writeFastHLine(curr - 1, 26, 3, COLOR_WHITE);
    display.writePixel(curr, 27, COLOR_WHITE);
  } else {
    cycle = (cycle + 1) % (SKIP_UPDATE);
    float b = sin(PI * cycle / (SKIP_UPDATE));  // ~1s cycle
    uint16_t color = display.color565(223 * b, 0, 191 * b);

    // If time negative, flash message every second
    display.writeFillRect(0, 25, 4, 4, color);
    display.writeFillRect(60, 25, 4, 4, color);

    display.setTextColor(color);
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

  skip_update = SKIP_UPDATE; 
  display.fillScreen(COLOR_BLACK);

  timer_settings_t timer = config.timers[cur_mode - 1];
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
  debugSim("Time: %02d:%02d\n", cur_min, abs(cur_time % 60));
}

void showLogo() {
  if (cur_mode == MODE_LOGO) {
    if (need_update) {
      // Randomize snowflakes
      for (uint8_t i = 0; i < SNOWFLAKE_COUNT; i++) {
        newSnowFlake(snowflakes[i]);
      }
      need_update = false;
    }

    if (skip_update > 0) {
      skip_update--;
      return;
    }

    skip_update = SKIP_UPDATE;
    drawBitmap(0, 0, bmp_snowcamp_64x32, 64, 32);
    cycle++;

    for (uint8_t i = 0; i < SNOWFLAKE_COUNT; i++) {
      if (snowflakes[i].y >= 0) {
        display.drawPixelRGB888(snowflakes[i].x, snowflakes[i].y, snowflakes[i].b * .5, snowflakes[i].b, snowflakes[i].b);
      }
      if ((cycle % snowflakes[i].s) == 0) {
        snowflakes[i].y++;
      }
      if (snowflakes[i].y == 32) {
        newSnowFlake(snowflakes[i]);
      }
    }

    display.showBuffer();
    debugSim("Animated logo\n");
  } else {
    if (need_update) {
      drawBitmap(0, 0, bmp_snowcamp_64x32, 64, 32);
      display.showBuffer();
      debugSim("Logo\n");
    }
    need_update = false;
  }
}

void showInfo() {
  if (need_update) {
    drawBitmap(0, 0, bmp_manual_64x32, 64, 32);
    display.showBuffer();
    debugSim("Info\n");
  }
  need_update = false;
}

void showConfig() {
  if (need_update) {
    display.fillScreen(COLOR_BLACK);
    display.setTextColor(COLOR_WHITE);
    display.setTextSize(0);
    display.setCursor(0, 12);
    display.print("WIFI");
    display.setCursor(29, 12);
    display.print("CONFIG");
    display.showBuffer();
    debugSim("Config\n");

    setupWifi();
  }
  need_update = false;
}

void setup() {
  Serial.begin(9600);
  Serial.println();

  loadConfig();

  brightness = config.brightness;
  initDisplay(brightness);

  pinMode(P_BUTTON, INPUT);
  button.attachClick(onPush);
  button.attachDoubleClick([]() { nextMode(); });
  button.attachMultiClick([]() {
    int clicks = button.getNumberClicks();
    if (clicks == 3) {
      brightness = (brightness + 64) % 256;
      display.setBrightness(brightness);
    } else if (clicks == 5) {
      fast_time = !fast_time;
    }
    resetScreensaverTimer();
  });
  button.setPressTicks(1000);
  button.attachLongPressStart(resetTimer);
  button.attachDuringLongPress(checkForConfigMode);

  showLogo();
  initWifi(DEBUG_LOGS);

  delay(3000);
  need_update = true;
}

void loop() {
  button.tick();

  if (cur_mode == MODE_INFO) {
    showInfo();
  } else if (cur_mode == MODE_LOGO) {
    showLogo();
  } else if (cur_mode == MODE_CONFIG) {
    showConfig();
  } else {
    showTimer();
  }
}
