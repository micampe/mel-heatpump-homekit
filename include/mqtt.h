#pragma once

#include <PubSubClient.h>

extern PubSubClient mqtt;

bool mqtt_is_configured();
bool mqtt_init(const char* name);
bool mqtt_connect();
void mqtt_loop();
