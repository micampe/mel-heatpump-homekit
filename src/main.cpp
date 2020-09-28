#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DoubleResetDetect.h>
#include <HeatPump.h>
#include <Ticker.h>
#include <WiFiManager.h>
#include <arduino_homekit_server.h>

#include "accessory.h"
#include "heatpump_client.h"

#define NAME_PREFIX "MIE_HVAC"

#define DRD_TIMEOUT 2.0
#define DRD_ADDRESS 0x00
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

HeatPump heatpump;
Ticker blinker;
Ticker timer;
homekit_server_t *homekit;

void blink() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));    
}

void stopBlinker() {
    blinker.detach();
    digitalWrite(LED_BUILTIN, HIGH);
}

void wifiConnectionFailed() {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
}

void wifiConfigModeCallback(WiFiManager *wifiManager) {
    blinker.attach(0.66, blink);
}

void homekit_setup(char *ssid) {
    char *name = strdup(ssid);
    accessory_name.value = HOMEKIT_STRING_CPP(name);
    arduino_homekit_setup(&accessory_config);
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(LED_BUILTIN, OUTPUT);

    char ssid[25];
    sprintf(ssid, NAME_PREFIX " %06x", ESP.getChipId());

    WiFiManager wifiManager;
    wifiManager.setAPCallback(wifiConfigModeCallback);
    wifiManager.setTimeout(120);

    if (drd.detect()) {
        Serial.println("Double reset detected");

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

    Serial.println("\nInitializing OTA...");
    // no mDNS, HomeKit will do that
    ArduinoOTA.begin(false);

    Serial.println("\nStarting KomeKit server...");
    delay(500);
    homekit_setup(ssid);
    homekit = arduino_homekit_get_running_server();
    if (!homekit->paired) {
        blinker.attach(1, blink);
        Serial.println("Waiting for accessory pairing");
        while (!homekit->paired) {
            arduino_homekit_loop();
            ArduinoOTA.handle();
            yield();
        }
        Serial.println("Paired, waiting for clients");
        while (arduino_homekit_connected_clients_count() == 0) {
            yield();
        }
        Serial.printf("%d clients connected.\n", arduino_homekit_connected_clients_count());
        blinker.detach();
        delay(500);
    }

    if (homekit->paired) {
        blinker.attach(0.33, blink);
        Serial.println("\nConnecting to heat pump...");
        delay(200);
        if (setupHeatPump()) {
            blinker.detach();
        } else {
            blinker.attach(0.1, blink);

            Serial.begin(115200);
            Serial.println();
            Serial.println("Heat pump connection failed");
        }
    }

    timer.once(2, stopBlinker);
}

void loop() {
    ArduinoOTA.handle();
    arduino_homekit_loop();
    if (heatpump.isConnected()) {
        heatpump.sync();
    }
}
