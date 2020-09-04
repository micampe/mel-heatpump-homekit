#ifndef DEHUMIDIFIER_SERVICE_H
#define DEHUMIDIFIER_SERVICE_H

#include <homekit/types.h>

#define DEHUMIDIFIER_INACTIVE 0
#define DEHUMIDIFIER_ACTIVE 1

#define DEHUMIDIFIER_CURRENT_STATE_INACTIVE 0
#define DEHUMIDIFIER_CURRENT_STATE_IDLE 1
#define DEHUMIDIFIER_CURRENT_STATE_HUMIDIFYING 2
#define DEHUMIDIFIER_CURRENT_STATE_DEHUMIDIFYING 3

extern homekit_characteristic_t ch_dehumidifier_active;
extern homekit_characteristic_t ch_dehumidifier_current_state;

extern homekit_service_t dehumidifier_service;

#endif

