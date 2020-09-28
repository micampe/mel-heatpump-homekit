#ifndef ACCESSORY_H
#define ACCESSORY_H

#include <homekit/homekit.h>
#include <homekit/types.h>

// Thermostat
extern homekit_characteristic_t ch_thermostat_current_temperature;
extern homekit_characteristic_t ch_thermostat_target_temperature;
extern homekit_characteristic_t ch_thermostat_current_heating_cooling_state;
extern homekit_characteristic_t ch_thermostat_target_heating_cooling_state;

// Dehumidifier
extern homekit_characteristic_t ch_dehumidifier_active;
extern homekit_characteristic_t ch_dehumidifier_current_state;
extern homekit_characteristic_t ch_dehumidifier_swing_mode;

// Fan
extern homekit_characteristic_t ch_fan_active;
extern homekit_characteristic_t ch_fan_rotation_speed;
extern homekit_characteristic_t ch_fan_swing_mode;
extern homekit_characteristic_t ch_fan_current_state;
extern homekit_characteristic_t ch_fan_target_state;

// Accessory
extern homekit_characteristic_t accessory_name;
extern homekit_server_config_t accessory_config;

#endif
