#include <Arduino.h>
#include <homekit/characteristics.h>
#include <homekit/homekit.h>
#include <homekit/types.h>

#include "dehumidifier_service.h"
#include "fan_service.h"
#include "log_c.h"
#include "thermostat_service.h"

#define ACCESSORY_NAME  ("HVAC")
#define ACCESSORY_SN  GIT_DESCRIBE
#define ACCESSORY_MANUFACTURER ("github.com micampe")
#define ACCESSORY_MODEL  ("ESP8266")
#define ACCESSORY_FW_REV  GIT_COMMITS

homekit_characteristic_t accessory_name = HOMEKIT_CHARACTERISTIC_(NAME, ACCESSORY_NAME);

void accessory_init() {
}

void accessory_identify(homekit_value_t _value) {
}

#define LED_ON LOW
#define LED_OFF HIGH
void set_led_on(homekit_value_t value) {
    if (value.bool_value) {
        digitalWrite(LED_BUILTIN, LED_ON);
    } else {
        digitalWrite(LED_BUILTIN, LED_OFF);
    }
}

homekit_characteristic_t ch_led_on = HOMEKIT_CHARACTERISTIC_(ON, false,
    .setter=set_led_on
);

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id = 1,
        .category = homekit_accessory_category_air_conditioner,
        .services = (homekit_service_t *[]) {
            HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t *[]) {
                &accessory_name,
                HOMEKIT_CHARACTERISTIC(MANUFACTURER, ACCESSORY_MANUFACTURER),
                HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, ACCESSORY_SN),
                HOMEKIT_CHARACTERISTIC(MODEL, ACCESSORY_MODEL),
                HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, ACCESSORY_FW_REV),
                HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                NULL
            }),
            &service_thermostat,
            &fan_service,
            &dehumidifier_service,
            HOMEKIT_SERVICE(LIGHTBULB, .characteristics = (homekit_characteristic_t *[]){
                HOMEKIT_CHARACTERISTIC(NAME, "Led"), 
                &ch_led_on, 
                NULL
            }),
            NULL
        }),
    NULL
};

homekit_server_config_t accessory_config = {
    .accessories = accessories,
    .password = "111-11-111",
};
