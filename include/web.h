#pragma once

#include <ESP8266WebServer.h>

struct Settings {
    char mqtt_server[32];
    uint16_t mqtt_port;
    char mqtt_temp[80];
    char mqtt_humidity[80];
    char mqtt_dew_point[80];
};

extern Settings settings;
extern ESP8266WebServer httpServer;

void initWeb(const char* hostname);
