#ifndef FAN_SERVICE_H
#define FAN_SERVICE_H

#include <homekit/types.h>

#define FAN_INACTIVE 0
#define FAN_ACTIVE 1

#define FAN_SWING_DISABLED 0
#define FAN_SWING_ENABLED 1

#define FAN_TARGET_STATE_MANUAL 0
#define FAN_TARGET_STATE_AUTO 1

#define FAN_CURRENT_STATE_INACTIVE 0
#define FAN_CURRENT_STATE_IDLE 1
#define FAN_CURRENT_STATE_BLOWING 2

#ifdef __cplusplus
extern "C" {
#endif

void set_fan_active(homekit_value_t value);
void set_fan_rotation_speed(homekit_value_t value);
void set_fan_swing_mode(homekit_value_t value);
void set_fan_target_mode(homekit_value_t value);

#ifdef __cplusplus
}
#endif

#endif
