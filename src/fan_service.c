#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#include "fan_service.h"
#include "log_c.h"

uint8_t fan_active = FAN_INACTIVE;
float fan_rotation_speed = 0;
uint8_t fan_swing_mode = FAN_SWING_DISABLED;
uint8_t fan_target_mode = 0;

void set_fan_active(homekit_value_t value) {
    fan_active = value.uint8_value;
    ch_fan_active.value.uint8_value = fan_active;
    DEBUG_LOG_VALUE_C("Fan active:", fan_active);
    if (fan_active == FAN_ACTIVE) {
        if (fan_rotation_speed <= 0) {
            DEBUG_LOG_C("Fan state: idle");
            homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(1));
        }
    } else {
        DEBUG_LOG_C("Fan state: inactive");
        homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(0));
    }
}

void set_fan_rotation_speed(homekit_value_t value) {
    fan_rotation_speed = value.float_value;
    // if (fan_rotation_speed < 8) {
    //     fan_rotation_speed = 0;
    // } else if (fan_rotation_speed < 16) {
    //     fan_rotation_speed = 12.5;
    // } else if (fan_rotation_speed < 37) {
    //     fan_rotation_speed = 25;
    // } else if (fan_rotation_speed < 62) {
    //     fan_rotation_speed = 50;
    // } else if (fan_rotation_speed < 87) {
    //     fan_rotation_speed = 75;
    // } else {
    //     fan_rotation_speed = 100;
    // }
    
    ch_fan_rotation_speed.value.float_value = fan_rotation_speed;
    DEBUG_LOG_VALUE_C("Fan speed:", fan_rotation_speed);
    if (fan_rotation_speed > 0 && fan_active == FAN_ACTIVE) {
        DEBUG_LOG_C("Set fan state: blowing");
        homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(2));
    } else if (fan_active == FAN_ACTIVE) {
        DEBUG_LOG_C("Set fan state: idle");
        homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(1));
    } else {
        DEBUG_LOG_C("Set fan state: inactive");
        homekit_characteristic_notify(&ch_fan_current_state, HOMEKIT_UINT8(0));
    }
}

void set_fan_swing_mode(homekit_value_t value) {
    fan_swing_mode = value.uint8_value;
    ch_fan_swing_mode.value.uint8_value = fan_swing_mode;
    DEBUG_LOG_VALUE_C("Fan swing:", fan_swing_mode);
}

void set_fan_target_mode(homekit_value_t value) {
    fan_target_mode = value.uint8_value;
    ch_fan_target_state.value.uint8_value = fan_target_mode;
    DEBUG_LOG_VALUE_C("Fan target mode:", fan_target_mode);
}

homekit_characteristic_t ch_fan_active = HOMEKIT_CHARACTERISTIC_(ACTIVE, FAN_INACTIVE,
        .setter = set_fan_active);
homekit_characteristic_t ch_fan_rotation_speed = HOMEKIT_CHARACTERISTIC_( ROTATION_SPEED, 2,
        .setter = set_fan_rotation_speed,
        .min_value = (float[]){0},
        .max_value = (float[]){5});
homekit_characteristic_t ch_fan_swing_mode = HOMEKIT_CHARACTERISTIC_(SWING_MODE, FAN_SWING_DISABLED,
        .setter = set_fan_swing_mode);
homekit_characteristic_t ch_fan_current_state = HOMEKIT_CHARACTERISTIC_(CURRENT_FAN_STATE, 0);
homekit_characteristic_t ch_fan_target_state = HOMEKIT_CHARACTERISTIC_(TARGET_FAN_STATE, 1,
        .setter = set_fan_target_mode);

homekit_service_t fan_service = HOMEKIT_SERVICE_(FAN2,
    .characteristics = (homekit_characteristic_t *[]) {
        HOMEKIT_CHARACTERISTIC(NAME, "Fan"),
        &ch_fan_active,
        &ch_fan_rotation_speed,
        &ch_fan_swing_mode,
        &ch_fan_current_state,
        &ch_fan_target_state,
        NULL
    });
