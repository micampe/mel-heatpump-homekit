#include <homekit/types.h>
#include <homekit/characteristics.h>

#include "accessory.h"
#include "debug.h"

void update_thermostat() {
    float current_temperature = ch_thermostat_current_temperature.value.float_value;

    uint8_t target_heating_cooling_state = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    float target_temperature = ch_thermostat_target_temperature.value.float_value;
    uint8_t current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;

    if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO 
            && current_temperature < target_temperature) {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO
            && current_temperature > target_temperature) {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL
            && current_temperature > target_temperature) {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT
            && current_temperature < target_temperature) {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
    }

    homekit_characteristic_notify(
            &ch_thermostat_current_heating_cooling_state,
            HOMEKIT_UINT8_CPP(current_heating_cooling_state));
}

extern "C" void set_thermostat_target_heating_cooling_state(homekit_value_t value) {
    uint8_t target_heating_cooling_state = value.uint8_value;
    ch_thermostat_target_heating_cooling_state.value.uint8_value = target_heating_cooling_state;
    MIE_LOG("HK set thermostat mode %d", target_heating_cooling_state);
    update_thermostat();
}

extern "C" void set_thermostat_target_temperature(homekit_value_t value) {
    float target_temperature = value.float_value;
    ch_thermostat_target_temperature.value.float_value = target_temperature;
    MIE_LOG("HK set target temperature %.2f", target_temperature);
    update_thermostat();
}

