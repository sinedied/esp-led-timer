#include <Arduino.h>
#include <OneButton.h>
#include "types.h"
#include "display.h"
#include "bitmaps.h"
#include "extra.h"
#include "config.h"
#include "network.h"

// #define DEBUG_SIM   1       // Simulate display on serial
#define DEBUG_FAST  0       // Accelerate time x100 for debug
#define STOP_AT     -9*60   // Stop overtime timer X seconds
#define HOLD_CONFIG 5       // Hold button for X seconds to enter config mode
#define HOLD_RESET  10      // Hold button for X seconds to reset all settings

#ifdef ESP32
  #define P_BUTTON    RX    // Pin for the hardware button
  #define SKIP_UPDATE 120   // Update every n cycles
#elif ESP8266
  #define P_BUTTON    D9    // Pin for the hardware button
  #define SKIP_UPDATE 60    // Update every n cycles
#endif

#ifdef DEBUG_SIM
  #define DEBUG_SIM_PRINTF(...)   Serial.printf(__VA_ARGS__)
  #undef SKIP_UPDATE
  #ifdef ESP32
    #define SKIP_UPDATE 1200
  #elif ESP8266
    #define SKIP_UPDATE 600
  #endif
#else
  #define DEBUG_SIM_PRINTF(...)
#endif


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
bool fast_time = DEBUG_FAST;
bool need_update = true;
bool has_switched_wifi = false;
uint16_t skip_update = 0;
int8_t prev_mode = MODE_UNDEF;
uint8_t progress_bar_warn[3];
uint8_t cycle = 0;
unsigned long press_start;
message_t message;
app_state_t state;
uint32_t timer_duration = 0;
unsigned long timer_start_time = 0;
unsigned int elapsed_time = 0; // in seconds
int8_t async_next_mode = state.cur_mode;
bool async_save_reboot = false;

static void nextMode(int8_t mode);

static void resetScreensaverTimer() {
  screensaver_ticker.detach();
  if (config.use_screensaver && state.cur_mode != MODE_LOGO && !state.timer_started) {
    screensaver_ticker.once(config.screensaver_time * 60, []() -> void {
      prev_mode = state.cur_mode;
      nextMode(MODE_LOGO);
    });
  }
}

void computeProgressBarWarnZones() {
  timer_settings_t timer = config.timers[state.cur_mode - 1];
  progress_bar_warn[0] = (timer.duration - timer.warn1) * DISPLAY_WIDTH / timer.duration;
  progress_bar_warn[1] = (timer.duration - timer.warn2) * DISPLAY_WIDTH / timer.duration;
  progress_bar_warn[2] = (timer.duration - timer.warn3) * DISPLAY_WIDTH / timer.duration;
}

static void resetTimer() {
  if (state.cur_mode >= MODE_TIMER_N) {
    state.cur_time = config.timers[state.cur_mode - 1].duration * 60;
    elapsed_time = 0;
    computeProgressBarWarnZones();
  }

  state.timer_started = false;
  timer_start_time = 0;
  time_ticker.detach();
  resetScreensaverTimer();
}

static void startTimer() {
  if (state.cur_mode >= MODE_TIMER_N && !state.timer_started) {
    state.timer_started = true;
    timer_duration = config.timers[state.cur_mode - 1].duration * 60;

    if (timer_start_time == 0) {
      timer_start_time = millis();
    } else {
      timer_start_time = millis() - (timer_duration - state.cur_time) * 1000;
    }

    time_ticker.attach(fast_time ? 0.01 : 0.25, []() -> void {
      if (fast_time) {
        --state.cur_time;
      } else {
        state.cur_time = timer_duration - (millis() - timer_start_time) / 1000 - elapsed_time;
      }

      if (state.cur_time <= STOP_AT) {
        resetTimer();
      }
    });
  }
}

static void stopTimer() {
  if (state.cur_mode >= MODE_TIMER_N && state.timer_started) {
    state.timer_started = false;
    time_ticker.detach();
    // Save already elapsed time
    elapsed_time += (millis() - timer_start_time) / 1000;
  }
}

static void checkForHoldActions() {
  uint32_t press_since = (millis() - press_start) / 1000;

  if (!has_switched_wifi && press_since > HOLD_CONFIG) {
    has_switched_wifi = true;

    // On ESP32, display need to be disabled during wifi activation
    displayUpdateEnable(false);
    enableWifi(!config.use_wifi);
    displayUpdateEnable(true);

    strcpy(message.title, "WIFI");
    sprintf(message.line1, "AP %s", config.use_wifi ? "on" : "off");
    strcpy(message.line2, config.use_wifi ? getIP().toString().c_str() : "");

    prev_mode = state.cur_mode;
    message.start_time = millis();
    nextMode(MODE_MESSAGE);
  }

  if (press_since > HOLD_RESET) {
    Serial.println("Resetting all settings");
    resetConfig();
    resetWifi();
    delay(1000);
    ESP.restart();
  }
}

static void nextMode(int8_t mode = MODE_UNDEF) {
  if (mode != MODE_UNDEF && (mode < MODE_MESSAGE || mode > MAX_TIMERS)) {
    Serial.printf("Invalid mode: %d\n", mode);
    return;
  }

  if (state.cur_mode == MODE_MESSAGE || (prev_mode != MODE_UNDEF && mode >= MODE_TIMER_N)) {
    state.cur_mode = prev_mode;
    prev_mode = MODE_UNDEF;
  } else {
    state.cur_mode = mode == MODE_UNDEF ? (state.cur_mode + 1) % (config.timer_count + 1) : mode;
  }

  resetTimer();
  async_next_mode = state.cur_mode;
  need_update = true;
  skip_update = 0;
  Serial.printf("Set mode: %d\n", state.cur_mode);
}

static void onPush() {
  if (state.cur_mode < MODE_TIMER_N) {
    nextMode(MODE_TIMER_N);
    return;
  }

  if (state.timer_started) {
    stopTimer();
  } else {
    startTimer();
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

void drawProgressbar(timer_settings_t& timer ) {
  int curr = DISPLAY_WIDTH - state.cur_time * DISPLAY_WIDTH / timer.duration / 60;
  int warn1 = (timer.duration - timer.warn1) * DISPLAY_WIDTH / timer.duration;
  int warn2 = (timer.duration - timer.warn2) * DISPLAY_WIDTH / timer.duration;
  int warn3 = (timer.duration - timer.warn3) * DISPLAY_WIDTH / timer.duration;

  if (state.cur_time > 0) {
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

  timer_settings_t timer = config.timers[state.cur_mode - 1];
  int cur_min = state.cur_time / 60;
  uint16_t cur_color = progress_colors[0];
  
  if (state.cur_time <= 0) {
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
  display.printf("%02d", abs(state.cur_time % 60));

  if (state.cur_time < 0) {
    display.writeFillRect(0, 12, 4, 2, cur_color);
  }

  drawProgressbar(timer);
  display.showBuffer();
  DEBUG_SIM_PRINTF("Time: %02d:%02d\n", cur_min, abs(state.cur_time % 60));
}

void showLogo() {
  if (state.cur_mode == MODE_LOGO) {
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
    DEBUG_SIM_PRINTF("Animated logo\n");
  } else {
    if (need_update) {
      drawBitmap(0, 0, bmp_snowcamp_64x32, 64, 32);
      display.showBuffer();
      DEBUG_SIM_PRINTF("Logo\n");
    }
    need_update = false;
  }
}

void showInfo() {
  if (need_update) {
    drawBitmap(0, 0, bmp_manual_64x32, 64, 32);
    display.showBuffer();
    DEBUG_SIM_PRINTF("Info\n");
  }
  need_update = false;
}

void showMessage() {
  uint32_t since = (millis() - message.start_time) / 1000;

  if (since > message.duration_sec) {
    Serial.println("Message timeout");
    nextMode();
    return;
  } else if (need_update) {
    display.fillScreen(COLOR_BLACK);
    display.setTextColor(COLOR_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(message.title);
    display.setTextColor(COLOR_YELLOW);
    display.setCursor(0, 10);
    display.print(message.line1);
    display.setCursor(0, 20);
    display.print(message.line2);
    display.showBuffer();
    DEBUG_SIM_PRINTF("Message: %s\n%s\n%s\n", message.title, message.line1, message.line2);
  }

  need_update = false;
}

void setup() {
  Serial.begin(9600);
  Serial.println();

  loadConfig();

  state.brightness = config.brightness;
  initDisplay(state.brightness);

  pinMode(P_BUTTON, INPUT);
  button.attachClick(onPush);
  button.attachDoubleClick([]() { nextMode(); });
  button.attachMultiClick([]() {
    int clicks = button.getNumberClicks();
    if (clicks == 3) {
      state.brightness = (state.brightness + 64) % 256;
      display.setBrightness(state.brightness);
    } else if (clicks == 5) {
      fast_time = !fast_time;
    }
    resetScreensaverTimer();
  });
  button.setPressMs(1000);
  button.attachLongPressStart([]() {
    resetTimer();
    press_start = millis();
    has_switched_wifi = false;
  });
  button.attachDuringLongPress(checkForHoldActions);

  control_callbacks_t callbacks;
  callbacks.startTimer = startTimer;
  callbacks.stopTimer = stopTimer;
  callbacks.resetTimer = resetTimer;
  callbacks.setBrightness = [&](uint8_t b) {
    state.brightness = b;
    display.setBrightness(state.brightness);
  };
  callbacks.setMode = [&](int8_t mode) {
    // Calling directly nextMode() async context will crash
    // the ESP when setting logo mode, not sure why
    async_next_mode = mode;
  };
  callbacks.resetScreensaverTimer = resetScreensaverTimer;
  callbacks.saveConfigAndReboot = [&]() {
    async_save_reboot = true;
  };
  // On ESP32, display need to be disabled during wifi activation
  displayUpdateEnable(false);
  initWifi(callbacks);
  displayUpdateEnable(true);

  showLogo();
  delay(3000);
  need_update = true;
}

void loop() {
  button.tick();

  if (async_save_reboot) {
    Serial.println("Async save config and reboot");
    // On ESP32, display need to be disabled during save config
    displayUpdateEnable(false);
    saveConfig();
    displayUpdateEnable(true);
    delay(1000);
    ESP.restart();
  }

  if (async_next_mode != state.cur_mode) {
    Serial.println("Async mode change");
    nextMode(async_next_mode);
  }

  if (state.cur_mode == MODE_INFO) {
    showInfo();
  } else if (state.cur_mode == MODE_LOGO) {
    showLogo();
  } else if (state.cur_mode == MODE_MESSAGE) {
    showMessage();
  } else {
    showTimer();
  }

  processServer();
}
