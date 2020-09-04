#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#include "fan_service.h"

uint8_t fan_active = FAN_INACTIVE;
float fan_rotation_speed = 0;
uint8_t fan_swing_mode = FAN_SWING_DISABLED;

void set_fan_active(homekit_value_t value) {
    fan_active = value.uint8_value;
    serial_log_value("Fan active: ", fan_active);
}

void set_fan_rotation_speed(homekit_value_t value) {
    fan_rotation_speed = value.float_value;
    if (fan_rotation_speed < 8) {
        fan_rotation_speed = 0;
    } else if (fan_rotation_speed < 16) {
        fan_rotation_speed = 12.5;
    } else if (fan_rotation_speed < 37) {
        fan_rotation_speed = 25;
    } else if (fan_rotation_speed < 62) {
        fan_rotation_speed = 50;
    } else if (fan_rotation_speed < 87) {
        fan_rotation_speed = 75;
    } else {
        fan_rotation_speed = 100;
    }
    
    serial_log_value("Fan speed: ", (int)fan_rotation_speed);
    homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT(fan_rotation_speed));
}

void set_fan_swing_mode(homekit_value_t value) {
    fan_swing_mode = value.uint8_value;
    serial_log_value("Fan swing: ", fan_swing_mode);
}

homekit_characteristic_t ch_fan_active = HOMEKIT_CHARACTERISTIC_(ACTIVE, FAN_INACTIVE,
    .setter = set_fan_active);
homekit_characteristic_t ch_fan_rotation_speed = HOMEKIT_CHARACTERISTIC_(ROTATION_SPEED, 10,
    .setter = set_fan_rotation_speed);
homekit_characteristic_t ch_fan_swing_mode = HOMEKIT_CHARACTERISTIC_(SWING_MODE, FAN_SWING_DISABLED,
    .setter = set_fan_swing_mode);

homekit_service_t fan_service = HOMEKIT_SERVICE_(FAN,
    .characteristics = (homekit_characteristic_t *[]) {
        HOMEKIT_CHARACTERISTIC(NAME, "Fan"),
        &ch_fan_active,
        &ch_fan_rotation_speed,
        &ch_fan_swing_mode,
        NULL
    });
