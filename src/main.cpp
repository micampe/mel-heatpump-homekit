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
    // setupHeatPump();
    homekit_setup();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    // heartbeat(10, true);
    handleOTA();
    homekit_loop();
    // updateHeatPump();
}
