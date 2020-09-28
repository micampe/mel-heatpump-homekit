#include <homekit/types.h>
#include <homekit/characteristics.h>

#include "accessory.h"

float target_temperature = 20;
uint8_t target_heating_cooling_state = HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;

void update_thermostat() {
    float current_temperature = ch_thermostat_current_temperature.value.float_value;

    uint8_t current_heating_cooling_state;
    if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO && current_temperature < target_temperature) {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO && current_temperature > target_temperature) {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL && current_temperature > target_temperature) {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT && current_temperature < target_temperature) {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
    } else {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
    }

    homekit_characteristic_notify(&ch_thermostat_current_heating_cooling_state, HOMEKIT_UINT8(current_heating_cooling_state));
}

void set_target_heating_cooling_state(homekit_value_t value) {
    target_heating_cooling_state = value.uint8_value;
    ch_thermostat_target_heating_cooling_state.value.uint8_value = target_heating_cooling_state;
    update_thermostat();
}

void set_target_temperature(homekit_value_t value) {
    target_temperature = value.float_value;
    ch_thermostat_target_temperature.value.uint8_value = target_temperature;
    update_thermostat();
}

