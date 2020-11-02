#include "homekit.h"

#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <arduino_homekit_server.h>
#include <homekit/characteristics.h>
#include <homekit/types.h>

#include "accessory.h"
#include "debug.h"
#include "heatpump_client.h"
#include "homekit.h"
#include "led_status_patterns.h"

static char serial[7];
static Ticker updateTicker;
static Ticker keepAliveTicker;

#define ANNOUNCE_INTERVAL 2

// throttle updates to the heat pump to try to send more settings at once and
// avoid conflicts when changing multiple settings from HomeKit
#define UPDATE_INTERVAL 5000

static heatpumpSettings _settingsForCurrentState() {
    heatpumpSettings settings;

    uint8_t therm_mode = ch_thermostat_target_heating_cooling_state.value.uint8_value;
    uint8_t fan_active = ch_fan_active.value.uint8_value;
    uint8_t dehum_active = ch_dehumidifier_active.value.uint8_value;

    settings.temperature = ch_thermostat_target_temperature.value.float_value;

    // I don't want to override, I want to keep what was there by default
    settings.mode = "AUTO";

    if (therm_mode == HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF && !fan_active && !dehum_active) {
        settings.power = "OFF";
    } else {
        settings.power = "ON";
        if (therm_mode != HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF) {
            if (therm_mode == HOMEKIT_TARGET_HEATING_COOLING_STATE_HEAT) {
                settings.mode = "HEAT";
            } else if (therm_mode == HOMEKIT_TARGET_HEATING_COOLING_STATE_COOL) {
                settings.mode = "COOL";
            } else {
                settings.mode = "AUTO";
            }
        } else if (dehum_active) {
            settings.mode = "DRY";
        } else if (fan_active) {
            settings.mode = "FAN";
        }
    }

    float speed = ch_fan_rotation_speed.value.float_value;
    if (ch_fan_target_state.value.uint8_value == 1) {
        settings.fan = "AUTO";
    } else if (speed > 0) {
        settings.fan = HP_SPEED(speed);
    }

    if (ch_fan_swing_mode.value.uint8_value == 1) {
        settings.vane = "SWING";
    } else {
        settings.vane = "AUTO";
    }

    if (ch_dehumidifier_swing_mode.value.uint8_value == 1) {
        settings.wideVane = "SWING";
    } else {
        settings.wideVane = "|";
    }

    return settings;
}

static void scheduleHeatPumpUpdate() {
    updateTicker.once_ms_scheduled(UPDATE_INTERVAL, [] {
        unsigned long start = millis();

        heatpumpSettings settings = _settingsForCurrentState();
        heatpump.setSettings(settings);
        MIE_LOG("⮕ HP updating power %s mode %s target %.1f fan %s v vane %s h vane %s",
                settings.power,
                settings.mode,
                settings.temperature,
                settings.fan,
                settings.vane,
                settings.wideVane);
        heatpump.update();

        (void)start;
        MIE_LOG("HP update %dms", millis() - start);
    });
}

static void _updateFanState(bool active) {
    accessory_set_uint8(&ch_fan_active, active, true);
    if (active) {
        uint8_t mode = ch_fan_target_state.value.uint8_value;
        float speed = ch_fan_rotation_speed.value.float_value;
        if (mode == 1) {
            accessory_set_float(&ch_fan_rotation_speed, HK_SPEED(AUTO_FAN_SPEED), true);
        } else if (speed < 20) {
            accessory_set_uint8(&ch_fan_target_state, 1, true);
            accessory_set_float(&ch_fan_rotation_speed, HK_SPEED(AUTO_FAN_SPEED), true);
        }
    } else {
        // _set_characteristic_float(&ch_fan_rotation_speed, 0, true);
    }
}

static void set_target_heating_cooling_state(homekit_value_t value) {
    uint8_t targetState = value.uint8_value;
    accessory_set_uint8(&ch_thermostat_target_heating_cooling_state, targetState, false);
    MIE_LOG("⬅ HK therm target state %d", targetState);

    bool active = targetState != HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF;
    _updateFanState(active);
    if (active) {
        accessory_set_uint8(&ch_dehumidifier_active, 0, true);
    }

    scheduleHeatPumpUpdate();
}

static void set_target_temperature(homekit_value_t value) {
    float targetTemperature = value.float_value;
    accessory_set_float(&ch_thermostat_target_temperature, targetTemperature, false);
    MIE_LOG("⬅ HK target temp %.1f", targetTemperature);

    scheduleHeatPumpUpdate();
}

static void set_dehumidifier_active(homekit_value_t value) {
    uint8_t active = value.uint8_value;
    accessory_set_uint8(&ch_dehumidifier_active, active, false);
    MIE_LOG("⬅ HK dehum active %d", active);

    _updateFanState(active);
    if (active) {
        accessory_set_uint8(&ch_thermostat_target_heating_cooling_state,
                HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF, true);
    }

    scheduleHeatPumpUpdate();
}

static void set_swing_horizontal(homekit_value_t value) {
    uint8_t swing = value.uint8_value;
    accessory_set_uint8(&ch_dehumidifier_swing_mode, swing, false);
    MIE_LOG("⬅ HK hor swing %d", swing);

    scheduleHeatPumpUpdate();
}

static void set_fan_active(homekit_value_t value) {
    uint8_t active = value.uint8_value;
    accessory_set_uint8(&ch_fan_active, active, false);
    MIE_LOG("⬅ HK fan active %d", active);

    if (!active) {
        accessory_set_uint8(&ch_dehumidifier_active, false, true);
        accessory_set_uint8(&ch_thermostat_target_heating_cooling_state,
                HOMEKIT_TARGET_HEATING_COOLING_STATE_OFF, true);
    }

    scheduleHeatPumpUpdate();
}

static void set_fan_speed(homekit_value_t value) {
    float speed = roundf(value.float_value);
    if (accessory_set_float(&ch_fan_rotation_speed, speed, false)) {
        MIE_LOG("⬅ HK fan speed %d", (int)speed);

        bool active = ch_fan_active.value.uint8_value;
        uint8_t mode = ch_fan_target_state.value.uint8_value;
        if (active && mode == 1) {
            MIE_LOG(" Fan is in auto mode, ignoring speed change");
            accessory_set_float(&ch_fan_rotation_speed, HK_SPEED(AUTO_FAN_SPEED), true);
        } else {
            accessory_set_uint8(&ch_fan_target_state, 0, true);
        }

        scheduleHeatPumpUpdate();
    }
}

static void set_fan_auto_mode(homekit_value_t value) {
    uint8_t mode = value.uint8_value;
    accessory_set_uint8(&ch_fan_target_state, mode, false);
    MIE_LOG("⬅ HK fan auto %d", mode);

    if (mode == 1) {
        accessory_set_float(&ch_fan_rotation_speed, HK_SPEED(AUTO_FAN_SPEED), true);
    }

    scheduleHeatPumpUpdate();
}

static void set_fan_swing(homekit_value_t value) {
    uint8_t swing = value.uint8_value;
    accessory_set_uint8(&ch_fan_swing_mode, swing, false);
    MIE_LOG("⬅ HK ver swing %d", swing);
    scheduleHeatPumpUpdate();
}


// the loop function will be called during the pairing process
void homekit_init(const char *ssid, std::function<void()> loop) {
    MIE_LOG("Starting HomeKit server...");

    sprintf(serial, "%06x", ESP.getChipId());
    accessory_serial.value = HOMEKIT_STRING_CPP(serial);
    accessory_name.value = HOMEKIT_STRING_CPP((char*)ssid);

    arduino_homekit_setup(&accessory_config);
    homekit_server_t *homekit = arduino_homekit_get_running_server();
    if (!homekit->paired) {
        MIE_LOG("Waiting for accessory pairing");

        led_status_set(&status_led_homekit_pairing);

        while (!homekit->paired) {
            loop();
            yield();
        }
        delay(100);
    }

    // this should only be updated when the accessory changes
    // or on firmware updates
    homekit_update_config_number();

    ch_thermostat_target_heating_cooling_state.setter = set_target_heating_cooling_state;
    ch_thermostat_target_temperature.setter = set_target_temperature;

    ch_dehumidifier_active.setter = set_dehumidifier_active;
    ch_dehumidifier_swing_mode.setter = set_swing_horizontal;

    ch_fan_active.setter = set_fan_active;
    ch_fan_rotation_speed.setter = set_fan_speed;
    ch_fan_target_state.setter = set_fan_auto_mode;
    ch_fan_swing_mode.setter = set_fan_swing;

    // Keep HomeKit connection alive
    // https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266/issues/9
    keepAliveTicker.attach_scheduled(ANNOUNCE_INTERVAL, []() {
        MDNS.announce();
    });
}

void homekit_loop() {
    arduino_homekit_loop();
}

int homekit_clients_count() {
    return arduino_homekit_connected_clients_count();
}
