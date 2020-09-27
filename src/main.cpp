#include <Arduino.h>
#include <DoubleResetDetect.h>
#include <HeatPump.h>
#include <Ticker.h>
#include <WiFiManager.h>
#include <arduino_homekit_server.h>

#include "heatpump_client.h"
#include "homekit.h"

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

void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(LED_BUILTIN, OUTPUT);
    blinker.attach(1, blink);

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

    Serial.println("\nStarting KomeKit server...");
    delay(500);
    homekit_setup(ssid);
    homekit = arduino_homekit_get_running_server();
    if (!homekit->paired) {
        Serial.println("Waiting for accessory pairing");
        while (!homekit->paired) {
            arduino_homekit_loop();
            yield();
        }
        Serial.println("Paired, waiting for clients");
        while (arduino_homekit_connected_clients_count() == 0) {
            yield();
        }
        Serial.printf("%d clients connected.\n", arduino_homekit_connected_clients_count());
        delay(1000);
    }

    if (homekit->paired) {
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
    homekit_loop();
    if (heatpump.isConnected()) {
        heatpump.sync();
    }
}
