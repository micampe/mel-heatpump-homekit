#include "homekit.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <arduino_homekit_server.h>
#include <homekit/types.h>

#include "accessory.h"
#include "debug.h"
#include "led_status_patterns.h"

void initHomeKitServer(const char *ssid) {
    Serial.println("Starting HomeKit server...");
    MIE_LOG("Starting HomeKit server...");
    accessory_name.value = HOMEKIT_STRING_CPP((char*)ssid);
    arduino_homekit_setup(&accessory_config);
    homekit_server_t *homekit = arduino_homekit_get_running_server();
    if (!homekit->paired) {
        Serial.println("Waiting for accessory pairing");
        MIE_LOG("Waiting for accessory pairing");

        led_status_set(&status_led_homekit_pairing);

        while (!homekit->paired) {
            arduino_homekit_loop();
            ArduinoOTA.handle();
            Debug.handle();
            yield();
        }
        delay(500);
    }
}
