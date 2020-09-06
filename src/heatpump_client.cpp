#include <Arduino.h>
#include <homekit/characteristics.h>

#include "heater_cooler_service.h"
#include "heatpump_client.h"
#include "homekit/homekit.h"
#include "log.h"
#include "thermostat_service.h"

HeatPump hp;

void updateHeatPumpMode(heatpumpSettings settings) {
    if (strncmp(settings.mode, "COOL", 4) == 0) {
        DEBUG_PRINTLN("Mode COOL");
        homekit_characteristic_notify(&ch_target_heater_cooler_state, HOMEKIT_UINT8_CPP(HEATER_COOLER_TARGET_STATE_COOL));
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL));
    } else if (strncmp(settings.mode, "HEAT", 4) == 0) {
        DEBUG_PRINTLN("Mode HEAT");
        homekit_characteristic_notify(&ch_target_heater_cooler_state, HOMEKIT_UINT8_CPP(HEATER_COOLER_TARGET_STATE_HEAT));
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT));
    } else if (strncmp(settings.mode, "AUTO", 4) == 0) {
        DEBUG_PRINTLN("Mode AUTO");
        homekit_characteristic_notify(&ch_target_heater_cooler_state, HOMEKIT_UINT8_CPP(HEATER_COOLER_TARGET_STATE_AUTO));
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO));
    } else if (strncmp(settings.mode, "DRY", 3) == 0) {
        DEBUG_PRINTLN("Mode DRY");
        homekit_characteristic_notify(&ch_heater_cooler_active, HOMEKIT_UINT8_CPP(HEATER_COOLER_INACTIVE));
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    } else if (strncmp(settings.mode, "FAN", 3) == 0) {
        DEBUG_PRINTLN("Mode FAN");
        homekit_characteristic_notify(&ch_heater_cooler_active, HOMEKIT_UINT8_CPP(HEATER_COOLER_INACTIVE));
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    }
}

void updateSwingMode(heatpumpSettings settings) {
    if (strncmp(settings.wideVane, "SWING", 5) == 0) {
        DEBUG_PRINTLN("Horizontal swing enabled");
        homekit_characteristic_notify(&ch_heater_cooler_swing_mode, HOMEKIT_UINT8_CPP(HEATER_COOLER_SWING_ENABLED));
    } else {
        DEBUG_PRINTLN("Horizontal swing disabled");
        homekit_characteristic_notify(&ch_heater_cooler_swing_mode, HOMEKIT_UINT8_CPP(HEATER_COOLER_SWING_DISABLED));
    }
}

void updateFanSpeed(heatpumpSettings settings) {
    if (strncmp(settings.fan, "AUTO", 5) == 0) {
        DEBUG_PRINTLN("Fan speed AUTO");
        homekit_characteristic_notify(&ch_heater_cooler_rotation_speed, HOMEKIT_FLOAT_CPP(12.5));
    } else if (strncmp(settings.fan, "QUIET", 5) == 0) {
        DEBUG_PRINTLN("Fan speed QUIET");
        homekit_characteristic_notify(&ch_heater_cooler_rotation_speed, HOMEKIT_FLOAT_CPP(12.5));
    } else if (strncmp(settings.fan, "1", 1) == 0) {
        DEBUG_PRINTLN("Fan speed 1");
        homekit_characteristic_notify(&ch_heater_cooler_rotation_speed, HOMEKIT_FLOAT_CPP(25));
    } else if (strncmp(settings.fan, "2", 1) == 0) {
        DEBUG_PRINTLN("Fan speed 2");
        homekit_characteristic_notify(&ch_heater_cooler_rotation_speed, HOMEKIT_FLOAT_CPP(50));
    } else if (strncmp(settings.fan, "3", 1) == 0) {
        DEBUG_PRINTLN("Fan speed 3");
        homekit_characteristic_notify(&ch_heater_cooler_rotation_speed, HOMEKIT_FLOAT_CPP(75));
    } else if (strncmp(settings.fan, "4", 1) == 0) {
        DEBUG_PRINTLN("Fan speed 4");
        homekit_characteristic_notify(&ch_heater_cooler_rotation_speed, HOMEKIT_FLOAT_CPP(100));
    }
}

void updateTemperatureSetting(heatpumpSettings settings) {
    DEBUG_PRINT("Setting thermostat target temperature: ");
    DEBUG_PRINTLN(settings.temperature);
    homekit_characteristic_notify(&ch_thermostat_target_temperature, HOMEKIT_FLOAT_CPP(settings.temperature));
    if (strncmp(settings.mode, "COOL", 4) == 0) {
        DEBUG_PRINT("Setting cooling threshold temperature: ");
        DEBUG_PRINTLN(settings.temperature);
        homekit_characteristic_notify(&ch_cooling_threshold_temperature, HOMEKIT_FLOAT_CPP(settings.temperature));
    } else if (strncmp(settings.mode, "HEAT", 4) == 0) {
        DEBUG_PRINT("Setting heating threshold temperature: ");
        DEBUG_PRINTLN(settings.temperature);
        homekit_characteristic_notify(&ch_heating_threshold_temperature, HOMEKIT_FLOAT_CPP(settings.temperature));
    } else if (strncmp(settings.mode, "AUTO", 4) == 0) {
        // maybe I can compare the settings to the current room temperature
        DEBUG_PRINTLN("Setting temperature range in AUTO mode not supported");
    }
}

void onConnect() {
    DEBUG_PRINTLN("Heat pump connected");
}

void update_thermostat_operating_status() {
    uint8_t target_heating_cooling_state = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    float target_temperature = ch_thermostat_target_temperature.value.float_value;
    float current_temperature = ch_thermostat_current_temperature.value.float_value;
    Serial.printf("Current temp: %.2\nf", current_temperature);
    Serial.printf("Target temp: %.2f\n", target_temperature);

    uint8_t current_heating_cooling_state;
    if (current_temperature < target_temperature) {
        if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO) {
            Serial.println("State auto heat");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
        } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT) {
            Serial.println("State heat");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
        } else {
            Serial.println("State OFF<");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
        }
    } else if (current_temperature > target_temperature) {
        if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO) {
            Serial.println("State auto cool");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
        } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL) {
            Serial.println("State cool");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
        } else {
            Serial.println("State OFF>");
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
        }
    } else {
        Serial.println("State OFF=");
        current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
    }

    homekit_characteristic_notify(&ch_thermostat_current_heating_cooling_state, HOMEKIT_UINT8_CPP(current_heating_cooling_state));
}

void settingsChanged() {
    DEBUG_PRINTLN("Settings changed");
    heatpumpSettings settings = hp.getSettings();
    if (strncmp(settings.power, "ON", 2) == 0) {
        DEBUG_PRINTLN("Heat pump is ON");
        updateHeatPumpMode(settings);
        updateSwingMode(settings);
        updateFanSpeed(settings);
        updateTemperatureSetting(settings);
    } else {
        DEBUG_PRINTLN("Heat pump is OFF");
        homekit_characteristic_notify(&ch_heater_cooler_active, HOMEKIT_UINT8_CPP(HEATER_COOLER_INACTIVE));
        homekit_characteristic_notify(&ch_thermostat_target_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF));
    }
}

void statusChanged(heatpumpStatus status) {
    DEBUG_PRINTLN("Status changed");
    homekit_characteristic_notify(&ch_heater_cooler_current_temperature, HOMEKIT_FLOAT_CPP(status.roomTemperature));
    homekit_characteristic_notify(&ch_thermostat_current_temperature, HOMEKIT_FLOAT_CPP(status.roomTemperature));
    if (status.operating) {
        homekit_characteristic_notify(&ch_heater_cooler_active, HOMEKIT_UINT8_CPP(HEATER_COOLER_ACTIVE));
        update_thermostat_operating_status();
    } else {
        homekit_characteristic_notify(&ch_heater_cooler_active, HOMEKIT_UINT8_CPP(HEATER_COOLER_INACTIVE));
        homekit_characteristic_notify(&ch_thermostat_current_heating_cooling_state, HOMEKIT_UINT8_CPP(HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF));
    }
}

void setupHeatPump() {
    hp.setOnConnectCallback(&onConnect);
    hp.setSettingsChangedCallback(&settingsChanged);
    hp.setStatusChangedCallback(&statusChanged);
    hp.enableAutoUpdate();
    hp.connect(&Serial);
}

void updateHeatPump() {
    hp.sync();
}
