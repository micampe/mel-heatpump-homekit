#ifndef THERMOSTAT_SERVICE_H
#define THERMOSTAT_SERVICE_H

#include <homekit/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void set_thermostat_target_heating_cooling_state(homekit_value_t value);
void set_thermostat_target_temperature(homekit_value_t value);

#ifdef __cplusplus
}
#endif

#endif
