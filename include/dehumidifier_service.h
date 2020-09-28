#ifndef DEHUMIDIFIER_SERVICE_H
#define DEHUMIDIFIER_SERVICE_H

#include <homekit/types.h>

#define DEHUMIDIFIER_INACTIVE 0
#define DEHUMIDIFIER_ACTIVE 1

#define DEHUMIDIFIER_SWING_DISABLED 0
#define DEHUMIDIFIER_SWING_ENABLED 1

#define DEHUMIDIFIER_CURRENT_STATE_INACTIVE 0
#define DEHUMIDIFIER_CURRENT_STATE_IDLE 1
// #define DEHUMIDIFIER_CURRENT_STATE_HUMIDIFYING 2
#define DEHUMIDIFIER_CURRENT_STATE_DEHUMIDIFYING 3

#ifdef __cplusplus
extern "C" {
#endif

void set_dehumidifier_active(homekit_value_t value);
void set_dehumidifier_swing_mode(homekit_value_t value);

#ifdef __cplusplus
}
#endif

#endif

