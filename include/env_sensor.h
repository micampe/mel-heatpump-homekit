#ifndef HUMIDITY_H
#define HUMIDITY_H

#include <stdbool.h>

extern char env_sensor_status[];

bool mqttIsConfigured();
void initEnvironmentReporting();

#endif
