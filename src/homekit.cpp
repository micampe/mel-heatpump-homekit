#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "accessory.h"
#include "homekit.h"
#include "log.h"

void homekit_setup() {
    accessory_init();

    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    int name_len = strnlen(accessory_name.value.string_value, UINT8_MAX) + 7; // 6 hex digits + 1 underbar
    char *name_value = (char *)malloc(name_len + 1);
    snprintf(name_value, name_len + 1, "%s_%02X%02X%02X", accessory_name.value.string_value, mac[3], mac[4], mac[5]);
    accessory_name.value = HOMEKIT_STRING_CPP(name_value);

    arduino_homekit_setup(&accessory_config);
}

void homekit_loop() {
    arduino_homekit_loop();
    static uint32_t next_heap_millis = 0;
    uint32_t time = millis();
    if (time > next_heap_millis) {
        DEBUG_LOG("heap: %d, sockets: %d\n", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
        next_heap_millis = time + 5000;
    }
}
