#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DoubleResetDetect.h>
#include <WiFiManager.h>
#include <arduino_homekit_server.h>
#include <led_status.h>

#include "accessory.h"
#include "debug.h"
#include "heatpump_client.h"
#include "homekit.h"
#include "humidity.h"
#include "led_status_patterns.h"
#include "ntp_clock.h"

#define NAME_PREFIX "Heat Pump "

#define DRD_TIMEOUT 2.0
#define DRD_ADDRESS 0x00
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

char ssid[25];

void wifiConnectionFailed() {
    led_status_signal(&status_led_error);
    Serial.println("WiFi connection failed, restarting");
    delay(1000);
    ESP.reset();
    delay(3000);
}

void wifiConfigModeCallback(WiFiManager *wifiManager) {
    led_status_set(&status_led_waiting_wifi);
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    led_status_init(LED_BUILTIN, false);

    sprintf(ssid, NAME_PREFIX "%06x", ESP.getChipId());

    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    WiFiManager wifiManager;
    wifiManager.setAPCallback(wifiConfigModeCallback);
    wifiManager.setTimeout(120);

    if (drd.detect()) {
        Serial.println("Double reset detected");
        led_status_signal(&status_led_double_reset);

        Serial.println("Clearing HomeKit config");
        homekit_storage_reset();

        Serial.println("Starting config portal");
        if (!wifiManager.startConfigPortal(ssid)) {
            wifiConnectionFailed();
        }
    } else {
        if (!wifiManager.autoConnect(ssid)) {
            wifiConnectionFailed();
        }
    }

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
