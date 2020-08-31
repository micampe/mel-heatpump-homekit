#include <Arduino.h>
#include <homekit/types.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <stdio.h>

#define ACCESSORY_NAME  ("HVAC")
#define ACCESSORY_SN  ("00001")
#define ACCESSORY_MANUFACTURER ("Arduino")
#define ACCESSORY_MODEL  ("ESP8266")
#define ACCESSORY_FW_REV  ("1.0")


uint8_t current_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
uint8_t target_heating_cooling_state = HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF;
float current_temperature = 20;
float target_temperature = 24;
uint8_t temperature_display_units = 0;


homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, ACCESSORY_NAME);

homekit_value_t get_current_heating_cooling_state() {
    return HOMEKIT_UINT8(current_heating_cooling_state);
}

homekit_characteristic_t ch_current_heating_cooling_state = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATING_COOLING_STATE, 
    HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF,
    .getter = get_current_heating_cooling_state
);


homekit_value_t get_target_heating_cooling_state() {
    return HOMEKIT_UINT8(target_heating_cooling_state);
}

void set_target_heating_cooling_state(homekit_value_t value) {
    if (value.format != homekit_format_int) {
        printf("Invalid target heating cooling state value format %d", value.format);
        return;
    }
    target_heating_cooling_state = value.int_value;
}

homekit_characteristic_t ch_target_heating_cooling_state = HOMEKIT_CHARACTERISTIC_(TARGET_HEATING_COOLING_STATE, 
    HOMEKIT_CURRENT_HEATING_COOLING_STATE_OFF,
    .getter = get_target_heating_cooling_state,
    .setter = set_target_heating_cooling_state
);



homekit_value_t get_current_temperature() {
    return HOMEKIT_FLOAT(current_temperature);
}

homekit_characteristic_t ch_current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE,
    0,
    .getter = get_current_temperature
);


homekit_value_t get_target_temperature() {
    return HOMEKIT_FLOAT(current_temperature);
}

void set_target_temperature(homekit_value_t value) {
    target_temperature = value.float_value;
}

homekit_characteristic_t ch_target_temperature = HOMEKIT_CHARACTERISTIC_(TARGET_TEMPERATURE,
    0,
    .getter = get_target_temperature,
    .setter = set_target_temperature
);


homekit_value_t get_temperature_display_units() {
    return HOMEKIT_UINT8(temperature_display_units);
}

void set_temperature_display_units(homekit_value_t value) {
    temperature_display_units = value.uint8_value;
}

homekit_characteristic_t ch_temperature_display_units = HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS,
    0,
    .getter = get_temperature_display_units,
    .setter = set_temperature_display_units
);


void accessory_identify(homekit_value_t _value) {
	printf("accessory identify\n");
}

homekit_characteristic_t led_on = HOMEKIT_CHARACTERISTIC_(ON, false,
	// .getter=led_on_get,
	// .setter=led_on_set
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
				NULL}),
			// HOMEKIT_SERVICE(LIGHTBULB, .primary = true, .characteristics = (homekit_characteristic_t *[]){
			// 	HOMEKIT_CHARACTERISTIC(NAME, "Led"), 
			// 	&led_on, 
			// 	NULL}),
            HOMEKIT_SERVICE(THERMOSTAT, .primary = true, .characteristics = (homekit_characteristic_t *[]) {
                HOMEKIT_CHARACTERISTIC(NAME, "Thermostat"),
                &ch_current_heating_cooling_state,
                &ch_target_heating_cooling_state,
                &ch_current_temperature,
                &ch_target_temperature,
                &ch_temperature_display_units,
                NULL}),
			NULL}),
	NULL
};

homekit_server_config_t config = {
	.accessories = accessories,
	.password = "111-11-111",
};

void accessory_init() {

}
