#include <Arduino.h>
#include <TelnetStream.h>

#include "OTA.h"
#include "heartbeat.h"
#include "heatpump_client.h"
#include "homekit.h"
#include "wifi.h"

#define NAME_PREFIX "HKM"

void setup() {
    Serial.begin(115200);
    Serial.println();
    TelnetStream.begin();

    pinMode(LED_BUILTIN, OUTPUT);

    setupWiFi(NAME_PREFIX);
    setupOTA(NAME_PREFIX);
    // setupHeatPump();
    homekit_setup();
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    // heartbeat(10, true);
    handleOTA();
    homekit_loop();
    // updateHeatPump();
}
