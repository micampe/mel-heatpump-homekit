#include <HeatPump.h>
#include <homekit/characteristics.h>

#include "accessory.h"
#include "heatpump_client.h"
#include "thermostat_service.h"
#include "fan_service.h"
#include "debug.h"

HeatPump heatpump;

void updateThermostatMode(heatpumpSettings settings) {
    uint8_t currentState = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    uint8_t state = currentState;

    if (strncmp(settings.mode, "COOL", 4) == 0) {
        state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
    } else if (strncmp(settings.mode, "HEAT", 4) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT;
    } else if (strncmp(settings.mode, "AUTO", 4) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO;
    } else if (strncmp(settings.mode, "DRY", 3) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;
    } else if (strncmp(settings.mode, "FAN", 3) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;
    }

    if (state != currentState) {
        ch_thermostat_target_heating_cooling_state.value.uint8_value = state;
        homekit_characteristic_notify(
                &ch_thermostat_target_heating_cooling_state,
                HOMEKIT_UINT8_CPP(state));
    }
}

void updateSwingMode(heatpumpSettings settings) {
    // horizontal swing
    if (strncmp(settings.wideVane, "SWING", 5) == 0) {
        ch_dehumidifier_swing_mode.value.uint8_value = FAN_SWING_ENABLED;
        homekit_characteristic_notify(
                &ch_dehumidifier_swing_mode,
                HOMEKIT_UINT8_CPP(FAN_SWING_ENABLED));
    } else {
        ch_dehumidifier_swing_mode.value.uint8_value = FAN_SWING_DISABLED;
        homekit_characteristic_notify(
                &ch_dehumidifier_swing_mode,
                HOMEKIT_UINT8_CPP(FAN_SWING_DISABLED));
    }

    // vertical swing
    if (strncmp(settings.vane, "SWING", 5) == 0) {
        ch_fan_swing_mode.value.uint8_value = FAN_SWING_ENABLED;
        homekit_characteristic_notify(&ch_fan_swing_mode, HOMEKIT_UINT8_CPP(FAN_SWING_ENABLED));
    } else {
        ch_fan_swing_mode.value.uint8_value = FAN_SWING_DISABLED;
        homekit_characteristic_notify(
                &ch_fan_swing_mode,
                HOMEKIT_UINT8_CPP(FAN_SWING_DISABLED));
    }
}

void updateFanSpeed(heatpumpSettings settings) {
    int speed = 0;
    uint8_t targetState = FAN_TARGET_STATE_MANUAL;

    if (strncmp(settings.fan, "QUIET", 5) == 0) {
        speed = 1;
    } else if (strncmp(settings.fan, "AUTO", 4) == 0) {
        targetState = FAN_TARGET_STATE_AUTO;
    } else {
        speed = 1 + strtol(settings.fan, NULL, 10);
    }

    if (speed > 0) {
        ch_fan_rotation_speed.value.float_value = speed;
        homekit_characteristic_notify(&ch_fan_rotation_speed, ch_fan_rotation_speed.value);
    }

    ch_fan_target_state.value.uint8_value = targetState;
    homekit_characteristic_notify(&ch_fan_target_state, ch_fan_target_state.value);
}

void updateTargetTemperature(heatpumpSettings settings) {
    ch_thermostat_target_temperature.value.float_value = settings.temperature;
    homekit_characteristic_notify(
            &ch_thermostat_target_temperature,
            HOMEKIT_FLOAT_CPP(settings.temperature));
}

void updateThermostatOperatingStatus(bool operating) {
    uint8_t target_heating_cooling_state = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    float target_temperature = ch_thermostat_target_temperature.value.float_value;
    float current_temperature = ch_thermostat_current_temperature.value.float_value;

    if (operating) {
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

        ch_thermostat_current_heating_cooling_state.value.uint8_value = current_heating_cooling_state;
        homekit_characteristic_notify(
                &ch_thermostat_current_heating_cooling_state,
                HOMEKIT_UINT8_CPP(current_heating_cooling_state));

        ch_fan_active.value.uint8_value = FAN_ACTIVE;
        homekit_characteristic_notify(
                &ch_fan_active,
                HOMEKIT_UINT8_CPP(FAN_ACTIVE));
    } else {
        ch_fan_active.value.uint8_value = FAN_INACTIVE;
        homekit_characteristic_notify(
                &ch_fan_active,
                HOMEKIT_UINT8_CPP(FAN_INACTIVE));
    }
}

void settingsChanged() {
    heatpumpSettings settings = heatpump.getSettings();
    MIE_LOG("HP settings updated: power %s; mode %s; target %.1f; fan %s; v vane %s; h vane %s",
            settings.power,
            settings.mode,
            settings.temperature,
            settings.fan,
            settings.vane,
            settings.wideVane);

    updateTargetTemperature(settings);
    updateThermostatMode(settings);
    updateSwingMode(settings);
    updateFanSpeed(settings);
}

void statusChanged(heatpumpStatus status) {
    MIE_LOG("HP Status updated: room temperature: %.1f; operating: %s",
            status.roomTemperature,
            status.operating ? "YES" : "NO");

    ch_thermostat_current_temperature.value.float_value = status.roomTemperature;
    homekit_characteristic_notify(
            &ch_thermostat_current_temperature,
            HOMEKIT_FLOAT_CPP(status.roomTemperature));
    updateThermostatOperatingStatus(status.operating);
}

void set_target_heating_cooling_state(homekit_value_t value) {
    uint8_t target_heating_cooling_state = value.uint8_value;
    ch_thermostat_target_heating_cooling_state.value.uint8_value = target_heating_cooling_state;
}

void set_target_temperature(homekit_value_t value) {
    float targetTemperature = value.float_value;
    ch_thermostat_target_temperature.value.uint8_value = targetTemperature;
    
    heatpump.setTemperature(targetTemperature);
    MIE_LOG("HK set target temperature to %.1f", targetTemperature);
}

bool setupHeatPump() {
    // ch_thermostat_target_heating_cooling_state.setter = set_target_heating_cooling_state;
    ch_thermostat_target_temperature.setter = set_target_temperature;

    heatpump.setSettingsChangedCallback(settingsChanged);
    heatpump.setStatusChangedCallback(statusChanged);

    heatpump.enableAutoUpdate();
    heatpump.enableExternalUpdate();

    return heatpump.connect(&Serial);
}

void updateHeatPump() {
    if (heatpump.isConnected()) {
        heatpump.sync();
    }
}
