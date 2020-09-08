#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#include "log_c.h"
#include "dehumidifier_service.h"
#include "fan_service.h"

uint8_t dehumidifier_active = DEHUMIDIFIER_INACTIVE;
uint8_t dehumidifier_current_state = DEHUMIDIFIER_CURRENT_STATE_INACTIVE;

void set_dehumidifier_active(homekit_value_t value) {
    dehumidifier_active = value.uint8_value;
    DEBUG_LOG_VALUE_C("Dehumidifier active:", dehumidifier_active);
    if (dehumidifier_active == DEHUMIDIFIER_ACTIVE) {
        homekit_characteristic_notify(&ch_dehumidifier_current_state, HOMEKIT_UINT8(DEHUMIDIFIER_CURRENT_STATE_DEHUMIDIFYING));
    } else {
        homekit_characteristic_notify(&ch_dehumidifier_current_state, HOMEKIT_UINT8(DEHUMIDIFIER_CURRENT_STATE_INACTIVE));
    }
}

homekit_characteristic_t ch_dehumidifier_active = HOMEKIT_CHARACTERISTIC_(ACTIVE, DEHUMIDIFIER_INACTIVE,
    .setter = set_dehumidifier_active);

homekit_characteristic_t ch_dehumidifier_current_state = HOMEKIT_CHARACTERISTIC_(
    CURRENT_HUMIDIFIER_DEHUMIDIFIER_STATE, 
    DEHUMIDIFIER_CURRENT_STATE_INACTIVE);

// setup target state characteristic to support dehumidifier only
homekit_characteristic_t ch_dehumidifier_target_state = HOMEKIT_CHARACTERISTIC_(
    TARGET_HUMIDIFIER_DEHUMIDIFIER_STATE,
    2,
    .min_value = (float[]) {2}, \
    .valid_values = { \
        .count = 1, \
        .values = (uint8_t[]) { 2 }, \
    });

homekit_service_t dehumidifier_service = HOMEKIT_SERVICE_(HUMIDIFIER_DEHUMIDIFIER,
    .characteristics = (homekit_characteristic_t *[]) {
        HOMEKIT_CHARACTERISTIC(NAME, "Dehumidifier"),
        &ch_dehumidifier_active,
        &ch_dehumidifier_current_state,
        HOMEKIT_CHARACTERISTIC(CURRENT_RELATIVE_HUMIDITY, 100),
        &ch_dehumidifier_target_state,
        NULL
    });

