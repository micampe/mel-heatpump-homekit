#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

uint8_t current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
uint8_t target_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
float target_temperature = 24;

void set_target_heating_cooling_state(homekit_value_t value) {
    target_heating_cooling_state = value.int_value;
    serial_print("Set target heating cooling state: ");
    serial_print_int(target_heating_cooling_state);
    serial_println("");
}

void set_target_temperature(homekit_value_t value) {
    target_temperature = value.float_value;
    serial_print("Set target temperature: ");
    serial_print_int((int)target_temperature);
    serial_println("");
}

homekit_characteristic_t ch_current_heating_cooling_state = HOMEKIT_CHARACTERISTIC_(
    CURRENT_HEATING_COOLING_STATE,
    HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF);

homekit_characteristic_t ch_target_heating_cooling_state = HOMEKIT_CHARACTERISTIC_(
    TARGET_HEATING_COOLING_STATE,
    HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF,
    .setter = set_target_heating_cooling_state);

homekit_characteristic_t ch_current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 25);

homekit_characteristic_t ch_target_temperature = HOMEKIT_CHARACTERISTIC_(
    TARGET_TEMPERATURE,
    21,
    .setter = set_target_temperature);

homekit_characteristic_t ch_temperature_display_units = HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS, 0);

homekit_service_t service_thermostat = HOMEKIT_SERVICE_(THERMOSTAT, .primary = true,
    .characteristics = (homekit_characteristic_t *[]) {
        HOMEKIT_CHARACTERISTIC(NAME, "Thermostat"),
        &ch_current_heating_cooling_state,
        &ch_target_heating_cooling_state,
        &ch_current_temperature,
        &ch_target_temperature,
        &ch_temperature_display_units,
        NULL
    });
