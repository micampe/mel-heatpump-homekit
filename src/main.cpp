#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DoubleResetDetect.h>
#include <WiFiManager.h>
#include <arduino_homekit_server.h>
#include <led_status.h>

#include "accessory.h"
#include "debug.h"
#include "heatpump_client.h"
#include "humidity.h"
#include "ntp_clock.h"
#include "led_status_patterns.h"

#define NAME_PREFIX "MIE Heat Pump "

#define DRD_TIMEOUT 2.0
#define DRD_ADDRESS 0x00
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

char ssid[25];
homekit_server_t *homekit;

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

void homekit_setup(char *ssid) {
    accessory_name.value = HOMEKIT_STRING_CPP(ssid);
    arduino_homekit_setup(&accessory_config);
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

    Serial.println("Initializing remote debug...");
    setupRemoteDebug(ssid);

    Serial.println("Initializing OTA...");
    MIE_LOG("Initializing OTA...");
    // no mDNS, HomeKit will do that
    ArduinoOTA.begin(false);

    Serial.println("Starting HomeKit server...");
    MIE_LOG("Starting HomeKit server...");
    delay(500);
    homekit_setup(ssid);
    homekit = arduino_homekit_get_running_server();
    if (!homekit->paired) {
        led_status_set(&status_led_homekit_pairing);
        Serial.println("Waiting for accessory pairing");
        MIE_LOG("Waiting for accessory pairing");
        while (!homekit->paired) {
            arduino_homekit_loop();
            ArduinoOTA.handle();
            Debug.handle();
            yield();
        }
        Serial.println("Paired, waiting for clients");
        MIE_LOG("Paired, waiting for clients");
        while (arduino_homekit_connected_clients_count() == 0) {
            yield();
        }
        Serial.printf("%d clients connected.\n", arduino_homekit_connected_clients_count());
        MIE_LOG("%d clients connected.", arduino_homekit_connected_clients_count());
        delay(500);
    }

    if (homekit->paired) {
        Serial.println("Connecting to heat pump... no more serial logging");
        MIE_LOG("Connecting to heat pump...");
        if (initHeatPump()) {
            initHumidityReporting();
        } else {
            led_status_signal(&status_led_error);
        }
    }
    led_status_done();
}

void loop() {
    ArduinoOTA.handle();
    arduino_homekit_loop();
    Debug.handle();
}
