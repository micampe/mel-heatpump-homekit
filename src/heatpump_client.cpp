#include <HeatPump.h>
#include <homekit/characteristics.h>
#include <Ticker.h>

#include "accessory.h"
#include "debug.h"
#include "heatpump_client.h"

HeatPump heatpump;
Ticker ticker;

void scheduleHeatPumpUpdate();


// --- Settings changes
void updateThermostatSettings(heatpumpSettings settings) {
    uint8_t state = HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;

    if (strncmp(settings.power, "OFF", 3) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;
    } else if (strncmp(settings.mode, "COOL", 4) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL;
    } else if (strncmp(settings.mode, "HEAT", 4) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT;
    } else if (strncmp(settings.mode, "AUTO", 4) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO;
    } else if (strncmp(settings.mode, "DRY", 3) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;
    } else if (strncmp(settings.mode, "FAN", 3) == 0) {
        state = HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;
    }

    ch_thermostat_target_heating_cooling_state.value.uint8_value = state;
    homekit_characteristic_notify(
            &ch_thermostat_target_heating_cooling_state,
            ch_thermostat_target_heating_cooling_state.value);

    ch_thermostat_target_temperature.value.uint8_value = settings.temperature;
    homekit_characteristic_notify(
            &ch_thermostat_target_temperature,
            ch_thermostat_target_temperature.value);
}

void updateFanSettings(heatpumpSettings settings) {
    int speed = 0;
    uint8_t targetState = FAN_TARGET_STATE_MANUAL;

    if (strncmp(settings.fan, "QUIET", 5) == 0) {
        speed = 1;
    } else if (strncmp(settings.fan, "AUTO", 4) == 0) {
        targetState = FAN_TARGET_STATE_AUTO;
        if (speed == 0) {
            speed = 2;
        }
    } else {
        speed = 1 + strtol(settings.fan, NULL, 10);
    }

    ch_fan_active.value.uint8_value = (heatpump.getPowerSettingBool() ? 1 : 0);
    homekit_characteristic_notify(&ch_fan_active, ch_fan_active.value);

    ch_fan_rotation_speed.value.float_value = (float)speed;
    homekit_characteristic_notify(&ch_fan_rotation_speed, ch_fan_rotation_speed.value);

    ch_fan_target_state.value.uint8_value = targetState;
    homekit_characteristic_notify(&ch_fan_target_state, ch_fan_target_state.value);

    // vertical swing
    if (strncmp(settings.vane, "SWING", 5) == 0) {
        ch_fan_swing_mode.value.uint8_value = 1;
    } else {
        ch_fan_swing_mode.value.uint8_value = 0;
    }
    homekit_characteristic_notify(&ch_fan_swing_mode, ch_fan_swing_mode.value);
}

void updateDehumidifierSettings(heatpumpSettings settings) {
    bool active = false;
    if (strncmp(settings.mode, "DRY", 3) == 0) {
        active = heatpump.getPowerSettingBool();
    }

    ch_dehumidifier_active.value.uint8_value = active ? 1 : 0;
    homekit_characteristic_notify(&ch_dehumidifier_active, ch_dehumidifier_active.value);

    // horizontal swing
    if (strncmp(settings.wideVane, "SWING", 5) == 0) {
        ch_dehumidifier_swing_mode.value.uint8_value = 1;
    } else {
        ch_dehumidifier_swing_mode.value.uint8_value = 0;
    }
    homekit_characteristic_notify(&ch_dehumidifier_swing_mode, ch_dehumidifier_swing_mode.value);
}

void settingsChanged() {
    heatpumpSettings settings = heatpump.getSettings();
    MIE_LOG("HP power %s; mode %s; target %.1f; fan %s; v vane %s; h vane %s",
            settings.power,
            settings.mode,
            settings.temperature,
            settings.fan,
            settings.vane,
            settings.wideVane);

    updateThermostatSettings(settings);
    updateFanSettings(settings);
    updateDehumidifierSettings(settings);
}


// --- Status updates
void updateThermostatOperatingStatus(bool operating) {
    uint8_t target_heating_cooling_state = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    float target_temperature = ch_thermostat_target_temperature.value.float_value;
    float current_temperature = ch_thermostat_current_temperature.value.float_value;

    uint8_t current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
    if (heatpump.getPowerSettingBool() && operating) {
        if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT) {
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
        } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL) {
            current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
        } else if (target_heating_cooling_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO) {
            if (current_temperature < target_temperature) {
                current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
            } else if (current_temperature > target_temperature) {
                current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
            }
        }
    }

    ch_thermostat_current_heating_cooling_state.value.uint8_value = current_heating_cooling_state;
    homekit_characteristic_notify(
            &ch_thermostat_current_heating_cooling_state,
            ch_thermostat_current_heating_cooling_state.value);
}

void updateFanOperatingStatus(bool operating) {
    if (operating) {
        ch_fan_current_state.value.uint8_value = FAN_CURRENT_STATE_BLOWING;
    } else {
        ch_fan_current_state.value.uint8_value = FAN_CURRENT_STATE_IDLE;
    }

    homekit_characteristic_notify(&ch_fan_current_state, ch_fan_current_state.value);
}

void updateDehumidifierOperatingStatus(bool operating) {
    if (strncmp(heatpump.getModeSetting(), "DRY", 3) == 0) {
        if (operating) {
            ch_dehumidifier_current_state.value = HOMEKIT_UINT8_CPP(DEHUMIDIFIER_CURRENT_STATE_DEHUMIDIFYING);
        } else {
            ch_dehumidifier_current_state.value = HOMEKIT_UINT8_CPP(DEHUMIDIFIER_CURRENT_STATE_IDLE);
        }
    } else {
        ch_dehumidifier_current_state.value = HOMEKIT_UINT8_CPP(DEHUMIDIFIER_CURRENT_STATE_INACTIVE);
    }

    homekit_characteristic_notify(&ch_dehumidifier_current_state, ch_dehumidifier_current_state.value);
}

void statusChanged(heatpumpStatus status) {
    MIE_LOG("HP room temp: %.1f; operating: %d", status.roomTemperature, status.operating);

    ch_thermostat_current_temperature.value.uint8_value = status.roomTemperature;
    homekit_characteristic_notify(
            &ch_thermostat_current_temperature,
            ch_thermostat_current_temperature.value);

    updateThermostatOperatingStatus(status.operating);
    updateFanOperatingStatus(status.operating);
    updateDehumidifierOperatingStatus(status.operating);
}


// --- HomeKit controls
void set_target_heating_cooling_state(homekit_value_t value) {
    uint8_t targetState = value.uint8_value;
    ch_thermostat_target_heating_cooling_state.value.uint8_value = targetState;

    switch (targetState) {
        // AUTO is not supported
        case HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL:
            heatpump.setPowerSetting(true);
            heatpump.setModeSetting("COOL");
            break;
        case HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT:
            heatpump.setPowerSetting(true);
            heatpump.setModeSetting("HEAT");
            break;
        case HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF:
            heatpump.setPowerSetting(false);
            break;
    }

    MIE_LOG("HK target state %d", targetState);
    scheduleHeatPumpUpdate();
}

void set_target_temperature(homekit_value_t value) {
    float targetTemperature = value.float_value;
    ch_thermostat_target_temperature.value.uint8_value = targetTemperature;

    heatpump.setTemperature(targetTemperature);

    MIE_LOG("HK target temperature %.1f", targetTemperature);
    scheduleHeatPumpUpdate();
}

void set_swing_horizontal(homekit_value_t value) {
    uint8_t swing = value.uint8_value;
    ch_dehumidifier_swing_mode.value.uint8_value = swing;

    if (swing == 1) {
        heatpump.setWideVaneSetting("SWING");
    } else {
        heatpump.setWideVaneSetting("AUTO");
    }

    MIE_LOG("HK hor swing %d", swing);
    scheduleHeatPumpUpdate();
}

void set_swing_vertical(homekit_value_t value) {
    uint8_t swing = value.uint8_value;
    ch_fan_swing_mode.value.uint8_value = swing;

    if (swing == 1) {
        heatpump.setVaneSetting("SWING");
    } else {
        heatpump.setVaneSetting("AUTO");
    }

    MIE_LOG("HK ver swing %d", swing);
    scheduleHeatPumpUpdate();
}

bool setupHeatPump() {
    ch_thermostat_target_heating_cooling_state.setter = set_target_heating_cooling_state;
    ch_thermostat_target_temperature.setter = set_target_temperature;

    ch_dehumidifier_swing_mode.setter = set_swing_horizontal;
    ch_fan_swing_mode.setter = set_swing_vertical;

    heatpump.setSettingsChangedCallback(settingsChanged);
    heatpump.setStatusChangedCallback(statusChanged);

    heatpump.enableExternalUpdate();
    heatpump.disableAutoUpdate();

    return heatpump.connect(&Serial);
}

#define UPDATE_INTERVAL 2
void scheduleHeatPumpUpdate() {
    ticker.once_scheduled(UPDATE_INTERVAL, [] {
        heatpump.update();
    });
}

void syncHeatPump() {
    if (heatpump.isConnected()) {
        heatpump.sync();
    }
}
