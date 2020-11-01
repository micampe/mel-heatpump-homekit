#pragma once

#include <stdbool.h>
#include <stdint.h>

struct Settings {
    char mqtt_server[32];
    uint16_t mqtt_port;
    char mqtt_temp[80];
    char mqtt_humidity[80];
    char mqtt_dew_point[80];
    bool debug;
};

extern Settings settings;

#define CONFIG_FILE "/config.json"

extern void settings_init();
