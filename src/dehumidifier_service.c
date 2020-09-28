#include <homekit/types.h>
#include <homekit/characteristics.h>

#include "accessory.h"
#include "dehumidifier_service.h"

uint8_t dehumidifier_active = DEHUMIDIFIER_INACTIVE;
uint8_t dehumidifier_current_state = DEHUMIDIFIER_CURRENT_STATE_INACTIVE;

void set_dehumidifier_active(homekit_value_t value) {
    dehumidifier_active = value.uint8_value;
    if (dehumidifier_active == DEHUMIDIFIER_ACTIVE) {
        homekit_characteristic_notify(&ch_dehumidifier_current_state, HOMEKIT_UINT8(DEHUMIDIFIER_CURRENT_STATE_DEHUMIDIFYING));
    } else {
        homekit_characteristic_notify(&ch_dehumidifier_current_state, HOMEKIT_UINT8(DEHUMIDIFIER_CURRENT_STATE_INACTIVE));
    }
}

