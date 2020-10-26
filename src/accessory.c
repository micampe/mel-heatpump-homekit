#include <Arduino.h>
#include <homekit/characteristics.h>
#include <homekit/homekit.h>
#include <homekit/types.h>

#include "accessory.h"
#include "custom_characteristics.h"

#define ACCESSORY_NAME  "Mitsubishi Air Conditioner"
#define ACCESSORY_MANUFACTURER "github.com micampe"
#define ACCESSORY_MODEL  GIT_DESCRIBE
#define ACCESSORY_FW_REV  GIT_COMMITS

void accessory_identify(homekit_value_t _value) {
}

// all setters needs to be initialized here, but
// they will be overridden by the heat pump client
void set_placeholder() {}

// Helpers

bool accessory_set_uint8(homekit_characteristic_t *characteristic, uint8_t value, bool notify) {
    if (characteristic->value.uint8_value != value) {
        characteristic->value.uint8_value = value;
        if (notify) {
            homekit_characteristic_notify(characteristic, characteristic->value);
        }
        return true;
    } else {
        return false;
    }
}

bool accessory_set_float(homekit_characteristic_t *characteristic, float value, bool notify) {
    if (characteristic->value.float_value != value) {
        characteristic->value.float_value = value;
        if (notify) {
            homekit_characteristic_notify(characteristic, characteristic->value);
        }
        return true;
    } else {
        return false;
    }
}


// Status LED

#define LED_ON LOW
#define LED_OFF HIGH
void set_led_on(homekit_value_t value) {
    if (value.bool_value) {
        digitalWrite(LED_BUILTIN, LED_ON);
    } else {
        digitalWrite(LED_BUILTIN, LED_OFF);
    }
}

homekit_characteristic_t ch_led_on = HOMEKIT_CHARACTERISTIC_(ON, false,
    .setter=set_led_on
);


// Thermostat

homekit_characteristic_t ch_thermostat_current_heating_cooling_state = HOMEKIT_CHARACTERISTIC_(
        CURRENT_HEATING_COOLING_STATE, HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF);
// disable auto mode
homekit_characteristic_t ch_thermostat_target_heating_cooling_state = HOMEKIT_CHARACTERISTIC_(
        TARGET_HEATING_COOLING_STATE,
        HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF,
        .setter = &set_placeholder,
        .valid_values = { \
            .count = 3, \
            .values = (uint8_t[]) { \
                HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF, \
                HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT, \
                HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL, \
                } \
        });
homekit_characteristic_t ch_thermostat_current_temperature = HOMEKIT_CHARACTERISTIC_(
        CURRENT_TEMPERATURE, 0);
homekit_characteristic_t ch_thermostat_target_temperature = HOMEKIT_CHARACTERISTIC_(
        TARGET_TEMPERATURE,
        16, // must always be in valid range
        .setter = &set_placeholder,
        .min_value = (float[]){16},
        .max_value = (float[]){31});
// only support celsius (this is only about the hardware, HomeKit will use the
// device region settings)
// it doesn't seem to do anything anyway, I was hoping it would remove the
// option in the accessory.
homekit_characteristic_t ch_temperature_display_units = HOMEKIT_CHARACTERISTIC_(
        TEMPERATURE_DISPLAY_UNITS, 
        0,
        .valid_values = { \
            .count = 1, \
            .values = (uint8_t[]) { 0 } \
        });

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

// Dehumidifier

homekit_characteristic_t ch_dehumidifier_active = HOMEKIT_CHARACTERISTIC_(
        ACTIVE, 0,
        .setter = &set_placeholder);

homekit_characteristic_t ch_dehumidifier_current_state = HOMEKIT_CHARACTERISTIC_(
        CURRENT_HUMIDIFIER_DEHUMIDIFIER_STATE, DEHUMIDIFIER_INACTIVE);

homekit_characteristic_t ch_dehumidifier_swing_mode = HOMEKIT_CHARACTERISTIC_(
        SWING_MODE, 0,
        .setter = &set_placeholder);

homekit_characteristic_t ch_dehumidifier_relative_humidity = HOMEKIT_CHARACTERISTIC_(
        CURRENT_RELATIVE_HUMIDITY, 0);

// support dehumidifier only
homekit_characteristic_t ch_dehumidifier_target_state = HOMEKIT_CHARACTERISTIC_(
        TARGET_HUMIDIFIER_DEHUMIDIFIER_STATE,
        2,
        .valid_values = { \
            .count = 1, \
            .values = (uint8_t[]) { 2 } \
        });

// Custom
homekit_characteristic_t ch_dew_point = HOMEKIT_CHARACTERISTIC_(DEW_POINT, 0);

homekit_service_t dehumidifier_service = HOMEKIT_SERVICE_(HUMIDIFIER_DEHUMIDIFIER,
    .characteristics = (homekit_characteristic_t *[]) {
        HOMEKIT_CHARACTERISTIC(NAME, "Dehumidifier"),
        &ch_dehumidifier_relative_humidity,
        &ch_dehumidifier_active,
        &ch_dehumidifier_current_state,
        &ch_dehumidifier_target_state,
        &ch_dehumidifier_swing_mode,
        &ch_dew_point,
        NULL
    });


// Fan

homekit_characteristic_t ch_fan_active = HOMEKIT_CHARACTERISTIC_(
        ACTIVE, 0,
        .setter = &set_placeholder);

homekit_characteristic_t ch_fan_rotation_speed = HOMEKIT_CHARACTERISTIC_(
        ROTATION_SPEED, 
        0,
        .setter = &set_placeholder,
        .min_step = (float[]){20});

homekit_characteristic_t ch_fan_swing_mode = HOMEKIT_CHARACTERISTIC_(
        SWING_MODE, 0,
        .setter = &set_placeholder);

homekit_characteristic_t ch_fan_current_state = HOMEKIT_CHARACTERISTIC_(
        CURRENT_FAN_STATE,
        FAN_CURRENT_STATE_INACTIVE);

homekit_characteristic_t ch_fan_target_state = HOMEKIT_CHARACTERISTIC_(
        TARGET_FAN_STATE,
        FAN_TARGET_STATE_AUTO,
        .setter = &set_placeholder);

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

// Accessories
homekit_characteristic_t accessory_name = HOMEKIT_CHARACTERISTIC_(NAME, ACCESSORY_NAME);
homekit_characteristic_t accessory_serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, "000");

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1,
        .category = homekit_accessory_category_air_conditioner,
        .services = (homekit_service_t *[]) {
            HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t *[]) {
                &accessory_name,
                &accessory_serial,
                HOMEKIT_CHARACTERISTIC(MANUFACTURER, ACCESSORY_MANUFACTURER),
                HOMEKIT_CHARACTERISTIC(MODEL, ACCESSORY_MODEL),
                HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, ACCESSORY_FW_REV),
                HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                NULL
            }),
            &service_thermostat,
            &fan_service,
            &dehumidifier_service,
            // HOMEKIT_SERVICE(LIGHTBULB, .characteristics = (homekit_characteristic_t *[]){
            //     HOMEKIT_CHARACTERISTIC(NAME, "Led"), 
            //     &ch_led_on, 
            //     NULL
            // }),
            NULL
        }),
    NULL
};

homekit_server_config_t accessory_config = {
    .accessories = accessories,
    .password = "111-11-111",
};
