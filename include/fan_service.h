#ifndef FAN_SERVICE_H
#define FAN_SERVICE_H

#include <homekit/types.h>

#define FAN_INACTIVE 0
#define FAN_ACTIVE 1

#define FAN_SWING_DISABLED 0
#define FAN_SWING_ENABLED 1

extern homekit_characteristic_t ch_fan_active;
extern homekit_characteristic_t ch_fan_rotation_speed;
extern homekit_characteristic_t ch_fan_swing_mode;

extern homekit_service_t fan_service;

#endif
