#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

#include "heater_cooler_service.h"

uint8_t heater_cooler_active = HEATER_COOLER_INACTIVE;
uint8_t current_heater_cooler_state = HEATER_COOLER_STATE_INACTIVE;
uint8_t target_heater_cooler_state = HEATER_COOLER_TARGET_STATE_AUTO;
float cooling_threshold_temperature = 26;
float heating_threshold_temperature = 16;

void update_state() {
    float current_temperature = ch_heater_cooler_current_temperature.value.float_value;

    if (heater_cooler_active == HEATER_COOLER_INACTIVE) {
        current_heater_cooler_state = HEATER_COOLER_STATE_INACTIVE;
    } else {
        current_heater_cooler_state = HEATER_COOLER_STATE_IDLE;
        if (target_heater_cooler_state == HEATER_COOLER_TARGET_STATE_AUTO) {
            serial_print("auto ");
            if (current_temperature > cooling_threshold_temperature) {
                current_heater_cooler_state = HEATER_COOLER_STATE_COOLING;
                serial_println("cooling");
            } else if (current_temperature < heating_threshold_temperature) {
                current_heater_cooler_state = HEATER_COOLER_STATE_HEATING;
                serial_println("heating");
            } else {
                serial_println("idle");
            }
        } else if (target_heater_cooler_state == HEATER_COOLER_TARGET_STATE_COOL && current_temperature > cooling_threshold_temperature) {
             current_heater_cooler_state = HEATER_COOLER_STATE_COOLING;
        } else if (target_heater_cooler_state == HEATER_COOLER_TARGET_STATE_HEAT && current_temperature < heating_threshold_temperature) {
            current_heater_cooler_state = HEATER_COOLER_STATE_HEATING;
        }
    }

    homekit_characteristic_notify(&ch_current_heater_cooler_state, HOMEKIT_UINT8(current_heater_cooler_state));
    serial_log_value("Current heater cooler state: ", current_heater_cooler_state);
}

void set_heater_cooler_active(homekit_value_t value) {
    heater_cooler_active = value.uint8_value;
    serial_log_value("Heater cooler active: ", heater_cooler_active);
    update_state();
}

void set_target_heater_cooler_state(homekit_value_t value) {
    target_heater_cooler_state = value.uint8_value;
    serial_log_value("Target heater cooler state: ", target_heater_cooler_state);
    update_state();
}

void set_cooling_threshold_temperature(homekit_value_t value) {
    cooling_threshold_temperature = value.float_value;
    serial_log_value("Cooling threshold: ", (int)cooling_threshold_temperature);
    update_state();
} 

void set_heating_threshold_temperature(homekit_value_t value) {
    heating_threshold_temperature = value.float_value;
    serial_log_value("Heating threshold: ", (int)heating_threshold_temperature);
    update_state();
}

homekit_characteristic_t ch_heater_cooler_active = HOMEKIT_CHARACTERISTIC_(ACTIVE, 0,
    .setter = set_heater_cooler_active);

homekit_characteristic_t ch_heater_cooler_current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 20);

homekit_characteristic_t ch_current_heater_cooler_state = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATER_COOLER_STATE, 1);

homekit_characteristic_t ch_target_heater_cooler_state = HOMEKIT_CHARACTERISTIC_(TARGET_HEATER_COOLER_STATE, 0,
    .setter = set_target_heater_cooler_state);

// we need to set heating/cooling thresholds extremes to the same values
// so the Home app correctly displays the current temperature in the picker
homekit_characteristic_t ch_cooling_threshold_temperature = HOMEKIT_CHARACTERISTIC_(COOLING_THRESHOLD_TEMPERATURE, 26,
    .setter = set_cooling_threshold_temperature,
    .min_value = (float[]){16},
    .max_value = (float[]){31});

homekit_characteristic_t ch_heating_threshold_temperature = HOMEKIT_CHARACTERISTIC_(HEATING_THRESHOLD_TEMPERATURE, 16,
    .setter = set_heating_threshold_temperature,
    .min_value = (float[]){16},
    .max_value = (float[]){31});

homekit_characteristic_t ch_heater_cooler_rotation_speed = HOMEKIT_CHARACTERISTIC_(ROTATION_SPEED, 30);

homekit_characteristic_t ch_heater_cooler_swing_mode = HOMEKIT_CHARACTERISTIC_(SWING_MODE, 0);

homekit_service_t service_heater_cooler = HOMEKIT_SERVICE_(HEATER_COOLER, .primary = true,
    .characteristics = (homekit_characteristic_t *[]) {
        HOMEKIT_CHARACTERISTIC(NAME, "Air Conditioner"),
        &ch_heater_cooler_active,
        &ch_heater_cooler_current_temperature,
        &ch_current_heater_cooler_state,
        &ch_target_heater_cooler_state,
        &ch_cooling_threshold_temperature,
        &ch_heating_threshold_temperature,
        &ch_heater_cooler_rotation_speed,
        &ch_heater_cooler_swing_mode,
        NULL
    });
