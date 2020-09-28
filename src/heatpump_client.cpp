#include <HeatPump.h>
#include <homekit/characteristics.h>

#include "accessory.h"
#include "heatpump_client.h"
#include "thermostat_service.h"
#include "fan_service.h"

HeatPump hp;

void updateHeatPumpMode(heatpumpSettings settings) {
    if (strncmp(settings.mode, "COOL", 4) == 0) {
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL));
    } else if (strncmp(settings.mode, "HEAT", 4) == 0) {
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT));
    } else if (strncmp(settings.mode, "AUTO", 4) == 0) {
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO));
    } else if (strncmp(settings.mode, "DRY", 3) == 0) {
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    } else if (strncmp(settings.mode, "FAN", 3) == 0) {
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    }
}

void updateSwingMode(heatpumpSettings settings) {
    if (strncmp(settings.wideVane, "SWING", 5) == 0) {
        homekit_characteristic_notify(&ch_fan_swing_mode, HOMEKIT_UINT8_CPP(FAN_SWING_ENABLED));
    } else {
        homekit_characteristic_notify(&ch_fan_swing_mode, HOMEKIT_UINT8_CPP(FAN_SWING_DISABLED));
    }
}

void updateFanSpeed(heatpumpSettings settings) {
    if (strncmp(settings.fan, "AUTO", 5) == 0) {
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(12.5));
    } else if (strncmp(settings.fan, "QUIET", 5) == 0) {
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(12.5));
    } else if (strncmp(settings.fan, "1", 1) == 0) {
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(25));
    } else if (strncmp(settings.fan, "2", 1) == 0) {
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(50));
    } else if (strncmp(settings.fan, "3", 1) == 0) {
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(75));
    } else if (strncmp(settings.fan, "4", 1) == 0) {
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(100));
    }
}

void updateTemperatureSetting(heatpumpSettings settings) {
    homekit_characteristic_notify(&ch_thermostat_target_temperature, HOMEKIT_FLOAT_CPP(settings.temperature));
}

void update_thermostat_operating_status() {
    uint8_t target_heating_cooling_state = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    float target_temperature = ch_thermostat_target_temperature.value.float_value;
    float current_temperature = ch_thermostat_current_temperature.value.float_value;

    uint8_t current_heating_cooling_state;
    if (current_temperature < target_temperature) {
        if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO) {
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
        } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT) {
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
        } else {
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
        }
    } else if (current_temperature > target_temperature) {
        if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO) {
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
        } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL) {
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
        } else {
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
        }
    } else {
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
    }

    homekit_characteristic_notify(&ch_thermostat_current_heating_cooling_state, HOMEKIT_UINT8_CPP(current_heating_cooling_state));
}

void settingsChanged() {
    heatpumpSettings settings = hp.getSettings();
    if (strncmp(settings.power, "ON", 2) == 0) {
        updateHeatPumpMode(settings);
        updateSwingMode(settings);
        updateFanSpeed(settings);
        updateTemperatureSetting(settings);
    } else {
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    }
}

void statusChanged(heatpumpStatus status) {
    homekit_characteristic_notify(&ch_thermostat_current_temperature, HOMEKIT_FLOAT_CPP(status.roomTemperature));
    if (status.operating) {
        update_thermostat_operating_status();
    } else {
        homekit_characteristic_notify(&ch_thermostat_current_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF));
    }
}

void set_target_heating_cooling_state(homekit_value_t value) {
    uint8_t target_heating_cooling_state = value.uint8_value;
    ch_thermostat_target_heating_cooling_state.value.uint8_value = target_heating_cooling_state;
}

void set_target_temperature(homekit_value_t value) {
    float target_temperature = value.float_value;
    ch_thermostat_target_temperature.value.uint8_value = target_temperature;
    
    hp.setTemperature(target_temperature);
}

bool setupHeatPump() {
    // ch_thermostat_target_heating_cooling_state.setter = set_target_heating_cooling_state;
    // ch_thermostat_target_temperature.setter = set_target_temperature;

    hp.setSettingsChangedCallback(settingsChanged);
    hp.setStatusChangedCallback(statusChanged);

    hp.enableAutoUpdate();
    hp.enableExternalUpdate();

    return hp.connect(&Serial);
}

void updateHeatPump() {
    if (hp.isConnected()) {
        hp.sync();
    }
}
