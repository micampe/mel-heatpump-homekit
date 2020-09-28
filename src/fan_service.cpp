#include <homekit/characteristics.h>
#include <homekit/types.h>

#include "fan_service.h"
#include "accessory.h"
#include "debug.h"

void set_fan_active(homekit_value_t value) {
    uint8_t fan_active = value.uint8_value;
    ch_fan_active.value.uint8_value = fan_active;
    MIE_LOG("HK Set fan active: %d", fan_active);
}

void set_fan_rotation_speed(homekit_value_t value) {
    float fan_rotation_speed = value.float_value;
    ch_fan_rotation_speed.value.float_value = fan_rotation_speed;
    MIE_LOG("HK Set fan speed: %d", (uint8_t)fan_rotation_speed);

    uint8_t fan_active = ch_fan_active.value.uint8_value;
    if (fan_rotation_speed > 0 && fan_active == FAN_ACTIVE) {
        homekit_characteristic_notify(
                &ch_fan_current_state,
                HOMEKIT_UINT8_CPP(FAN_CURRENT_STATE_BLOWING));
    } else if (fan_active == FAN_ACTIVE) {
        homekit_characteristic_notify(
                &ch_fan_current_state,
                HOMEKIT_UINT8_CPP(FAN_CURRENT_STATE_IDLE));
    } else {
        homekit_characteristic_notify(
                &ch_fan_current_state,
                HOMEKIT_UINT8_CPP(FAN_CURRENT_STATE_INACTIVE));
    }
}

void set_fan_swing_mode(homekit_value_t value) {
    uint8_t fan_swing_mode = value.uint8_value;
    ch_fan_swing_mode.value.uint8_value = fan_swing_mode;
    MIE_LOG("HK Set fan swing: %d", fan_swing_mode);
}

void set_fan_target_mode(homekit_value_t value) {
    uint8_t fan_target_mode = value.uint8_value;
    ch_fan_target_state.value.uint8_value = fan_target_mode;
    MIE_LOG("HK Set fan auto: %d", fan_target_mode);
}

