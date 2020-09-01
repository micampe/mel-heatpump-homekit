#include <Arduino.h>
#include <homekit/characteristics.h>
#include <homekit/homekit.h>
#include <homekit/types.h>

#include "serial_bridge.h"
#include "thermostat_service.h"

#define ACCESSORY_NAME  ("HVAC")
#define ACCESSORY_SN  ("00001")
#define ACCESSORY_MANUFACTURER ("Arduino")
#define ACCESSORY_MODEL  ("ESP8266")
#define ACCESSORY_FW_REV  ("1.0")

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, ACCESSORY_NAME);

void accessory_init() {
	serial_println("accessory init");
}

void accessory_identify(homekit_value_t _value) {
	serial_println("accessory identify");
}

);

homekit_accessory_t *accessories[] = {
	HOMEKIT_ACCESSORY(.id = 1,
        .category = homekit_accessory_category_air_conditioner,
		.services = (homekit_service_t *[]) {
			HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t *[]) {
				&name,
				HOMEKIT_CHARACTERISTIC(MANUFACTURER, ACCESSORY_MANUFACTURER),
				HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, ACCESSORY_SN),
				HOMEKIT_CHARACTERISTIC(MODEL, ACCESSORY_MODEL),
				HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, ACCESSORY_FW_REV),
				HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
				NULL
            }),
            &service_thermostat,
			NULL}),
	NULL
};

homekit_server_config_t config = {
	.accessories = accessories,
	.password = "111-11-111",
};
