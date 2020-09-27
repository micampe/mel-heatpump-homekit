#include <Arduino.h>
#include <homekit/characteristics.h>

#include "heatpump_client.h"
#include "log.h"
#include "thermostat_service.h"
#include "fan_service.h"

HeatPump hp;

void updateHeatPumpMode(heatpumpSettings settings) {
    if (strncmp(settings.mode, "COOL", 4) == 0) {
        DEBUG_LOG("Mode COOL\n");
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL));
    } else if (strncmp(settings.mode, "HEAT", 4) == 0) {
        DEBUG_LOG("Mode HEAT\n");
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT));
    } else if (strncmp(settings.mode, "AUTO", 4) == 0) {
        DEBUG_LOG("Mode AUTO\n");
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO));
    } else if (strncmp(settings.mode, "DRY", 3) == 0) {
        DEBUG_LOG("Mode DRY\n");
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    } else if (strncmp(settings.mode, "FAN", 3) == 0) {
        DEBUG_LOG("Mode FAN\n");
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    }
}

void updateSwingMode(heatpumpSettings settings) {
    if (strncmp(settings.wideVane, "SWING", 5) == 0) {
        DEBUG_LOG("Swing enabled\n");
        homekit_characteristic_notify(&ch_fan_swing_mode, HOMEKIT_UINT8_CPP(FAN_SWING_ENABLED));
    } else {
        DEBUG_LOG("Swing disabled\n");
        homekit_characteristic_notify(&ch_fan_swing_mode, HOMEKIT_UINT8_CPP(FAN_SWING_DISABLED));
    }
}

void updateFanSpeed(heatpumpSettings settings) {
    if (strncmp(settings.fan, "AUTO", 5) == 0) {
        DEBUG_LOG("Fan speed AUTO\n");
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(12.5));
    } else if (strncmp(settings.fan, "QUIET", 5) == 0) {
        DEBUG_LOG("Fan speed QUIET\n");
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(12.5));
    } else if (strncmp(settings.fan, "1", 1) == 0) {
        DEBUG_LOG("Fan speed 1\n");
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(25));
    } else if (strncmp(settings.fan, "2", 1) == 0) {
        DEBUG_LOG("Fan speed 2\n");
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(50));
    } else if (strncmp(settings.fan, "3", 1) == 0) {
        DEBUG_LOG("Fan speed 3\n");
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(75));
    } else if (strncmp(settings.fan, "4", 1) == 0) {
        DEBUG_LOG("Fan speed 4\n");
        homekit_characteristic_notify(&ch_fan_rotation_speed, HOMEKIT_FLOAT_CPP(100));
    }
}

void updateTemperatureSetting(heatpumpSettings settings) {
    DEBUG_LOG("Target temperature: %.2f\n", settings.temperature);
    homekit_characteristic_notify(&ch_thermostat_target_temperature, HOMEKIT_FLOAT_CPP(settings.temperature));
}

void onConnect() {
    DEBUG_LOG("Heat pump connected\n");
}

void update_thermostat_operating_status() {
    uint8_t target_heating_cooling_state = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    float target_temperature = ch_thermostat_target_temperature.value.float_value;
    float current_temperature = ch_thermostat_current_temperature.value.float_value;
    DEBUG_LOG("Current temp: %.2f\n", current_temperature);
    DEBUG_LOG("Target temp: %.2f\n", target_temperature);

    uint8_t current_heating_cooling_state;
    if (current_temperature < target_temperature) {
        if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO) {
            DEBUG_LOG("State auto heat\n");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
        } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT) {
            DEBUG_LOG("State heat\n");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
        } else {
            DEBUG_LOG("State OFF<\n");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
        }
    } else if (current_temperature > target_temperature) {
        if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO) {
            DEBUG_LOG("State auto cool\n");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
        } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL) {
            DEBUG_LOG("State cool\n");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
        } else {
            DEBUG_LOG("State OFF>\n");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
        }
    } else {
        DEBUG_LOG("State OFF=\n");
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
    }

    homekit_characteristic_notify(&ch_thermostat_current_heating_cooling_state, HOMEKIT_UINT8_CPP(current_heating_cooling_state));
}

void settingsChanged() {
    DEBUG_LOG("Settings changed\n");
    heatpumpSettings settings = hp.getSettings();
    if (strncmp(settings.power, "ON", 2) == 0) {
        DEBUG_LOG("Heat pump is ON\n");
        updateHeatPumpMode(settings);
        updateSwingMode(settings);
        updateFanSpeed(settings);
        updateTemperatureSetting(settings);
    } else {
        DEBUG_LOG("Heat pump is OFF\n");
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    }
}

void statusChanged(heatpumpStatus status) {
    DEBUG_LOG("Status changed\n");
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
    DEBUG_LOG("HK set target state: %d\n", target_heating_cooling_state);
}

void set_target_temperature(homekit_value_t value) {
    float target_temperature = value.float_value;
    ch_thermostat_target_temperature.value.uint8_value = target_temperature;
    DEBUG_LOG("HK set target temperature: %.2f\n", target_temperature);
    
    hp.setTemperature(target_temperature);
    hp.update();
}

void setupHeatPump() {
#if CONNECT_HEAT_PUMP
    ch_thermostat_target_heating_cooling_state.setter = set_target_heating_cooling_state;
    ch_thermostat_target_temperature.setter = set_target_temperature;

    hp.setOnConnectCallback(onConnect);
    hp.setSettingsChangedCallback(settingsChanged);
    hp.setStatusChangedCallback(statusChanged);
    hp.enableAutoUpdate();
    hp.enableExternalUpdate();
    if (hp.connect(&Serial)) {
        DEBUG_LOG("HP connection successful\n");
    } else {
        DEBUG_LOG("HP connection failed\n");
    }
#else
    DEBUG_LOG("HP connection disabled\n");
#endif
}

void updateHeatPump() {
#if CONNECT_HEAT_PUMP
    if (!hp.isConnected()) {
        return;
    }

    static unsigned long hpTick = millis();
    if (millis() > hpTick + 5000) {
        hp.sync();
        heatpumpStatus status = hp.getStatus();
        DEBUG_LOG("Room temperature: %.2f, operating: %d\n", status.roomTemperature, status.operating);
        hpTick = millis();
    }
#endif
}
