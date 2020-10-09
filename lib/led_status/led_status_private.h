#pragma once

#include <stdint.h>

typedef struct {
    int n;
    int16_t *delay;
} led_status_pattern_t;


#define LED_STATUS_PATTERN(...) { \
    .n = sizeof((int16_t[])__VA_ARGS__) / sizeof(int16_t), \
    .delay = (int16_t[])__VA_ARGS__, \
}
