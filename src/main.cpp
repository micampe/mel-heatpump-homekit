#include <Arduino.h>
#include <DoubleResetDetect.h>
#include <TelnetStream.h>

#include "OTA.h"
#include "heartbeat.h"
#include "heatpump_client.h"
#include "homekit.h"
#include "wifi.h"
#include "log.h"

#define NAME_PREFIX "HKM"

#define DRD_TIMEOUT 2.0
#define DRD_ADDRESS 0x00
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

void setup() {
    Serial.begin(115200);
    Serial.println();
    TelnetStream.begin();

    if (drd.detect()) {
        DEBUG_LOG("*** Double reset detected\n");
        DEBUG_LOG("Clearing HomeKit config\n");
        homekit_storage_reset();
    }

    setupWiFi(NAME_PREFIX);
    setupOTA(NAME_PREFIX);

    // Delay so I have time to connect to the log
    for (int i = 0; i < 100; i++) {
        delay(100);
        if (i % 10 == 0) {
            DEBUG_LOG("%d ", (int)(10 - i / 10));
        }
    }
    DEBUG_LOG("\n");

    DEBUG_LOG("Initializing HomeKit...\n");
    homekit_setup();

    DEBUG_LOG("Connecting to heat pump...\n");
    setupHeatPump();

    DEBUG_LOG("Setup done.\n");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    // heartbeat(10, false);
    handleOTA();
    homekit_loop();
    updateHeatPump();
}
