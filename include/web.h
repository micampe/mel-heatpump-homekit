#pragma once

#include <ESP8266WebServer.h>

extern ESP8266WebServer httpServer;

void web_init(const char* hostname);
