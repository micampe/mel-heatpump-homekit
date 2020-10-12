#include <stdlib.h>

#include <Arduino.h>
#include <Ticker.h>

#include "led_status_private.h"

#define ABS(x) (((x) < 0) ? -(x) : (x))

typedef struct {
    uint8_t gpio;
    uint8_t active;

    int n;
    const led_status_pattern_t *pattern;
    const led_status_pattern_t *signal_pattern;
} led_status_t;

led_status_t status;
Ticker ticker;


static void led_status_write(bool on) {
    digitalWrite(status.gpio, on ? status.active : !status.active);
}

static void led_status_tick() {
    const led_status_pattern_t *p = status.signal_pattern ? status.signal_pattern : status.pattern;
    if (!p) {
        ticker.detach();
        led_status_write(false);
        return;
    }

    led_status_write(p->delay[status.n] > 0);
    ticker.once_ms(ABS(p->delay[status.n]), led_status_tick);

    status.n = (status.n + 1) % p->n;
    if (status.signal_pattern && status.n == 0) {
        status.signal_pattern = NULL;
    }
}

void led_status_init(uint8_t gpio, bool active_level) {
    status.gpio = gpio;
    status.active = active_level;

    pinMode(gpio, OUTPUT);

    led_status_write(false);
}

void led_status_done() {
    status.pattern = NULL;
}

void led_status_set(const led_status_pattern_t *pattern) {
    status.pattern = pattern;

    if (!status.signal_pattern) {
        status.n = 0;
        led_status_tick();
    }
}

void led_status_signal(const led_status_pattern_t *pattern) {
    if (!status.signal_pattern && !pattern)
        return;

    status.signal_pattern = pattern;
    status.n = 0;  // whether signal pattern is NULL or not, just reset the state
    led_status_tick();
}
