#include <homekit/characteristics.h>
#include <homekit/types.h>

#include "accessory.h"
#include "fan_service.h"

uint8_t fan_active = FAN_INACTIVE;
float fan_rotation_speed = 0;
uint8_t fan_swing_mode = FAN_SWING_DISABLED;
uint8_t fan_target_mode = 0;

void set_fan_active(homekit_value_t value) {
    fan_active = value.uint8_value;
    ch_fan_active.value.uint8_value = fan_active;
    if (fan_active == FAN_ACTIVE) {
        if (fan_rotation_speed <= 0) {
            homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(1));
        }
    } else {
        homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(0));
    }
}

void set_fan_rotation_speed(homekit_value_t value) {
    fan_rotation_speed = value.float_value;
    ch_fan_rotation_speed.value.float_value = fan_rotation_speed;
    if (fan_rotation_speed > 0 && fan_active == FAN_ACTIVE) {
        homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(2));
    } else if (fan_active == FAN_ACTIVE) {
        homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(1));
    } else {
        homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(0));
    }
}

void set_fan_swing_mode(homekit_value_t value) {
    fan_swing_mode = value.uint8_value;
    ch_fan_swing_mode.value.uint8_value = fan_swing_mode;
}

void set_fan_target_mode(homekit_value_t value) {
    fan_target_mode = value.uint8_value;
    ch_fan_target_state.value.uint8_value = fan_target_mode;
}

