#ifndef HUMIDITY_H
#define HUMIDITY_H

#include <MQTTClient.h>

extern MQTTClient mqtt;
extern char env_sensor_status[];

bool mqttIsConfigured();
void initEnvironmentReporting(const char* ssid);

#endif
