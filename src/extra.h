#ifndef __EXTRA_H
#define __EXTRA_H

#include <Arduino.h>

#define SNOWFLAKE_COUNT   32

struct snowflake_t {
  uint8_t x;
  int8_t y;
  uint8_t b;  // brightness
  uint8_t s;  // fall speed, in cycles to wait
};

static snowflake_t snowflakes[SNOWFLAKE_COUNT];

void newSnowFlake(snowflake_t& snowflake) {
  snowflake.x = random(0, 64);
  snowflake.y = random(0, 32) - 32;
  snowflake.b = random(0, 255);
  snowflake.s = random(2, 8);
}

#endif // __EXTRA_H