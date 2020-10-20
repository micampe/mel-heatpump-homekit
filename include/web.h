#pragma once

#include <ESP8266WebServer.h>

extern ESP8266WebServer httpServer;

void initWeb(const char* hostname);
