#include <homekit/types.h>
#include <homekit/characteristics.h>

#include "accessory.h"
#include "dehumidifier_service.h"
#include "debug.h"

void set_dehumidifier_active(homekit_value_t value) {
    uint8_t dehumidifier_active = value.uint8_value;
    ch_dehumidifier_active.value.uint8_value = dehumidifier_active;
    MIE_LOG("HK Set dehumidifier_active active: %d", dehumidifier_active);
    
    if (dehumidifier_active == DEHUMIDIFIER_ACTIVE) {
        homekit_characteristic_notify(
                &ch_dehumidifier_current_state,
                HOMEKIT_UINT8_CPP(DEHUMIDIFIER_CURRENT_STATE_DEHUMIDIFYING));
    } else {
        homekit_characteristic_notify(
                &ch_dehumidifier_current_state,
                HOMEKIT_UINT8_CPP(DEHUMIDIFIER_CURRENT_STATE_INACTIVE));
    }
}

void set_dehumidifier_swing_mode(homekit_value_t value) {
    uint8_t swing_mode = value.uint8_value;
    ch_dehumidifier_swing_mode.value.uint8_value = swing_mode;
    MIE_LOG("HK Set dehumidifier swing: %d", swing_mode);
}
