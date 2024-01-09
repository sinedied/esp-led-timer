#ifndef __TYPES_H
#define __TYPES_H

#include <Arduino.h>

enum app_mode_t {
  MODE_UNDEF = -100,
  MODE_MESSAGE = -2,
  MODE_INFO = -1,
  MODE_LOGO = 0,
  MODE_TIMER_N,
};

struct message_t {
  char title[12] = "";
  char line1[24] = "";
  char line2[24] = "";
  uint8_t duration_sec = 5;
  time_t start_time;
};

#endif // __TYPES_H