#include "led_status_patterns.h"

led_status_pattern_t status_led_error = LED_STATUS_PATTERN({
        100, -100, 100, -100, 100, -100, 100, -100, 100, -1000});
led_status_pattern_t status_led_double_reset = LED_STATUS_PATTERN({
        200, -200, 200, -200, 200, -1500});
led_status_pattern_t status_led_waiting_wifi = LED_STATUS_PATTERN({
        500, -500, 500, -1500});
led_status_pattern_t status_led_homekit_pairing = LED_STATUS_PATTERN({
        1000, -1000});
