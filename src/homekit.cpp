#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "accessory.h"
#include "homekit.h"
#include "log.h"

void homekit_setup(char *ssid) {
    accessory_init();
    char *name = strdup(ssid);
    accessory_name.value = HOMEKIT_STRING_CPP(name);
    arduino_homekit_setup(&accessory_config);
}

void homekit_loop() {
    arduino_homekit_loop();
}
