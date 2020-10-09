#include <Arduino.h>
#include <ArduinoOTA.h>
#include <arduino_homekit_server.h>
#include <led_status.h>

#include "debug.h"
#include "heatpump_client.h"
#include "homekit.h"
#include "humidity.h"
#include "led_status_patterns.h"
#include "ntp_clock.h"
#include "wifi_manager.h"

#define NAME_PREFIX "Heat Pump "

char ssid[25];

void setup() {
    Serial.begin(115200);
    Serial.println();

    led_status_init(LED_BUILTIN, false);

    sprintf(ssid, NAME_PREFIX "%06x", ESP.getChipId());

    initWiFiManager(ssid);
    initNTPClock();
    initRemoteDebug(ssid);
    MIE_LOG("Initializing OTA...");
    ArduinoOTA.begin(false);
    initHomeKitServer(ssid);

    if (initHeatPump()) {
        initHumidityReporting();
    } else {
        led_status_signal(&status_led_error);
    }

    led_status_done();
}

void loop() {
    ArduinoOTA.handle();
    arduino_homekit_loop();
    Debug.handle();
}
