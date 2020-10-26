#include <HeatPump.h>
#include <homekit/characteristics.h>
#include <Ticker.h>

#include "accessory.h"
#include "debug.h"
#include "heatpump_client.h"
#include "homekit.h"

HeatPump heatpump;

static Ticker syncTicker;

// slower sync seems to help in avoiding unresponsive homekit accessory
#define SYNC_INTERVAL 3000


// --- Settings changes
static void updateThermostatSettings(heatpumpSettings settings) {
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

    bool changed = false;
    changed |= accessory_set_uint8(&ch_thermostat_target_heating_cooling_state, state, true);
    changed |= accessory_set_float(&ch_thermostat_target_temperature, settings.temperature, true);

    if (changed) {
        MIE_LOG(" ⮕ HK therm target mode %d temp %.1f",
                ch_thermostat_target_heating_cooling_state.value.uint8_value,
                ch_thermostat_target_temperature.value.float_value);
    }
}

static void updateFanSettings(heatpumpSettings settings) {
    int speed = 0;
    uint8_t targetState = FAN_TARGET_STATE_MANUAL;

    if (strncmp(settings.fan, "QUIET", 5) == 0) {
        speed = 1;
    } else if (strncmp(settings.fan, "AUTO", 4) == 0) {
        targetState = FAN_TARGET_STATE_AUTO;
        speed = AUTO_FAN_SPEED;
    } else {
        speed = 1 + strtol(settings.fan, NULL, 10);
    }

    bool changed = false;
    changed |= accessory_set_uint8(&ch_fan_active, (heatpump.getPowerSettingBool() ? 1 : 0), true);
    changed |= accessory_set_float(&ch_fan_rotation_speed, HK_SPEED(speed), true);
    changed |= accessory_set_uint8(&ch_fan_target_state, targetState, true);

    // vertical swing
    if (strncmp(settings.vane, "SWING", 5) == 0) {
        changed |= accessory_set_uint8(&ch_fan_swing_mode, 1, true);
    } else {
        changed |= accessory_set_uint8(&ch_fan_swing_mode, 0, true);
    }

    if (changed) {
        MIE_LOG(" ⮕ HK fan active %d speed %d auto %d swing %d",
                ch_fan_active.value.uint8_value,
                (int)ch_fan_rotation_speed.value.float_value,
                ch_fan_target_state.value.uint8_value,
                ch_fan_swing_mode.value.uint8_value);
    }
}

static void updateDehumidifierSettings(heatpumpSettings settings) {
    uint8_t active = 0;
    if (strncmp(settings.mode, "DRY", 3) == 0) {
        active = heatpump.getPowerSettingBool() ? 1 : 0;
    }

    bool changed = false;
    changed |= accessory_set_uint8(&ch_dehumidifier_active, active, false);

    // horizontal swing
    if (strncmp(settings.wideVane, "SWING", 5) == 0) {
        changed |= accessory_set_uint8(&ch_dehumidifier_swing_mode, 1, true);
    } else {
        changed |= accessory_set_uint8(&ch_dehumidifier_swing_mode, 0, true);
    }

    if (changed) {
        MIE_LOG(" ⮕ HK dehum active %d swing %d",
                ch_dehumidifier_active.value.uint8_value,
                ch_dehumidifier_swing_mode.value.uint8_value);
    }
}


// --- Status updates
static void updateThermostatOperatingStatus(bool operating) {
    uint8_t target_state = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    float target_temperature = ch_thermostat_target_temperature.value.float_value;
    float current_temperature = ch_thermostat_current_temperature.value.float_value;

    uint8_t current_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
    const char* mode = PSTR("idle");
    if (heatpump.getPowerSettingBool() && operating) {
        if (target_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT) {
            current_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
            mode = PSTR("heating");
        } else if (target_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL) {
            current_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
            mode = PSTR("cooling");
        } else if (target_state == HOMEKIT_TARGET_HEATING_COOLING_STATE_AUTO) {
            if (current_temperature < target_temperature) {
                current_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_HEAT;
                mode = PSTR("heating");
            } else if (current_temperature > target_temperature) {
                current_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_COOL;
                mode = PSTR("cooling");
            }
        }
    }

    bool changed = false;
    changed |= accessory_set_uint8(&ch_thermostat_current_heating_cooling_state, current_state, true);

    if (changed) {
        (void)mode;
        MIE_LOG(" ⮕ HK therm %s temp %.1f", mode, current_temperature);
    }
}

static void updateFanOperatingStatus(bool operating) {
    bool changed = false;
    const char* status;
    if (heatpump.getPowerSettingBool() == false) {
        changed |= accessory_set_uint8(&ch_fan_current_state, FAN_CURRENT_STATE_INACTIVE, true);
        status = PSTR("inactive");
    } else if (operating) {
        changed |= accessory_set_uint8(&ch_fan_current_state, FAN_CURRENT_STATE_BLOWING, true);
        status = PSTR("blowing");
    } else {
        changed |= accessory_set_uint8(&ch_fan_current_state, FAN_CURRENT_STATE_IDLE, true);
        status = PSTR("idle");
    }

    if (changed) {
        (void)status;
        MIE_LOG(" ⮕ HK fan %s", status);
    }
}

static void updateDehumidifierOperatingStatus(bool operating) {
    bool changed = false;
    if (strncmp(heatpump.getModeSetting(), "DRY", 3) == 0) {
        if (heatpump.getPowerSettingBool() == false) {
            changed |= accessory_set_uint8(&ch_dehumidifier_current_state, DEHUMIDIFIER_INACTIVE, false);
        } else if (operating) {
            changed |= accessory_set_uint8(&ch_dehumidifier_current_state, DEHUMIDIFIER_DEHUMIDIFYING, false);
        } else {
            changed |= accessory_set_uint8(&ch_dehumidifier_current_state, DEHUMIDIFIER_IDLE, false);
        }
    } else {
        changed |= accessory_set_uint8(&ch_dehumidifier_current_state, DEHUMIDIFIER_INACTIVE, false);
    }

    if (changed) {
        MIE_LOG(" ⮕ HK dehum state %d", ch_dehumidifier_current_state.value.uint8_value);
    }
}


static void settingsChanged() {
    heatpumpSettings settings = heatpump.getSettings();
    MIE_LOG("⬅ HP power %s mode %s target %.1f fan %s v vane %s h vane %s",
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

static void statusChanged(heatpumpStatus status) {
    MIE_LOG("⬅ HP room temp %.1f op %d cmp %d",
            status.roomTemperature,
            status.operating,
            status.compressorFrequency);

    accessory_set_float(&ch_thermostat_current_temperature, status.roomTemperature, false);

    updateThermostatOperatingStatus(status.operating);
    updateFanOperatingStatus(status.operating);
    updateDehumidifierOperatingStatus(status.operating);
}


bool heatpump_init() {
    MIE_LOG("Connecting to heat pump... disabling serial logging");
    Debug.enableSerial(false);

    heatpump.setSettingsChangedCallback(settingsChanged);
    heatpump.setStatusChangedCallback(statusChanged);

    heatpump.enableExternalUpdate();
    heatpump.disableAutoUpdate();

    syncTicker.attach_ms_scheduled(SYNC_INTERVAL, [] {
        unsigned long start = millis();
        if (heatpump.isConnected()) {
            heatpump.sync();
            int diff = millis() - start;
            if (diff > 150) {
                MIE_LOG("HP sync %dms", diff);
            }
        }
    });

    delay(100);
    if (heatpump.connect(&Serial)) {
        MIE_LOG("Heat pump connected");
        return true;
    } else {
        Serial.begin(115200);
        Debug.enableSerial(true);
        MIE_LOG("Heat pump connection failed, serial logging resumed");
        return false;
    }
}
