#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#include "log_c.h"
#include "thermostat_service.h"

float target_temperature = 23.5;
uint8_t target_heating_cooling_state = HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;

void update_thermostat() {
    float current_temperature = ch_thermostat_current_temperature.value.float_value;
    DEBUG_LOG_VALUE_C("current temp:", current_temperature);
    DEBUG_LOG_VALUE_C("target temp:", target_temperature);

    uint8_t current_heating_cooling_state;
    if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO && current_temperature < target_temperature) {
        DEBUG_LOG_C("State auto heat");
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO && current_temperature > target_temperature) {
        DEBUG_LOG_C("State auto cool");
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL && current_temperature > target_temperature) {
        DEBUG_LOG_C("State cool");
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
    } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT && current_temperature < target_temperature) {
        DEBUG_LOG_C("State heat");
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
    } else {
        DEBUG_LOG_C("State OFF");
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
    }

    homekit_characteristic_notify(&ch_thermostat_current_heating_cooling_state, HOMEKIT_UINT8(current_heating_cooling_state));
}

void set_target_heating_cooling_state(homekit_value_t value) {
    target_heating_cooling_state = value.uint8_value;
    ch_thermostat_target_heating_cooling_state.value.uint8_value = target_heating_cooling_state;
    DEBUG_LOG_VALUE_C("Set target heating cooling state:", target_heating_cooling_state);
    update_thermostat();
}

void set_target_temperature(homekit_value_t value) {
    target_temperature = value.float_value;
    ch_thermostat_target_temperature.value.uint8_value = target_temperature;
    DEBUG_LOG_VALUE_C("Set target temperature:", target_temperature);
    update_thermostat();
}

homekit_characteristic_t ch_thermostat_current_heating_cooling_state = HOMEKIT_CHARACTERISTIC_(
    CURRENT_HEATING_COOLING_STATE,
    HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF);

homekit_characteristic_t ch_thermostat_target_heating_cooling_state = HOMEKIT_CHARACTERISTIC_(
    TARGET_HEATING_COOLING_STATE,
    HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF,
    .setter = set_target_heating_cooling_state);

homekit_characteristic_t ch_thermostat_current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 20);

homekit_characteristic_t ch_thermostat_target_temperature = HOMEKIT_CHARACTERISTIC_(
    TARGET_TEMPERATURE,
    18,
    .setter = set_target_temperature,
    .min_value = (float[]){16},
    .max_value = (float[]){31});

homekit_characteristic_t ch_temperature_display_units = HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS, 0);

homekit_service_t service_thermostat = HOMEKIT_SERVICE_(THERMOSTAT, .primary = true,
    .characteristics = (homekit_characteristic_t *[]) {
        HOMEKIT_CHARACTERISTIC(NAME, "Thermostat"),
        &ch_thermostat_current_heating_cooling_state,
        &ch_thermostat_target_heating_cooling_state,
        &ch_thermostat_current_temperature,
        &ch_thermostat_target_temperature,
        &ch_temperature_display_units,
        NULL
    });
