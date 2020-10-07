#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DoubleResetDetect.h>
#include <HeatPump.h>
#include <Ticker.h>
#include <TZ.h>
#include <WiFiManager.h>
#include <arduino_homekit_server.h>
#include <coredecls.h>

#include "accessory.h"
#include "debug.h"
#include "heatpump_client.h"
#include "humidity.h"

#define NAME_PREFIX "MIE Heat Pump "

#define DRD_TIMEOUT 2.0
#define DRD_ADDRESS 0x00
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

char ssid[25];
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
    accessory_name.value = HOMEKIT_STRING_CPP(ssid);
    arduino_homekit_setup(&accessory_config);
}

bool timeWasSet = false;

void setClock() {
    configTime(TZ_Etc_UTC, "pool.ntp.org");

    settimeofday_cb([] {
        timeWasSet = true;
        MIE_LOG("NTP sync");
    });
    Serial.print("Waiting for NTP time sync...");
    while (!timeWasSet) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    time_t now;
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(LED_BUILTIN, OUTPUT);

    sprintf(ssid, NAME_PREFIX "%06x", ESP.getChipId());

    WiFi.setSleepMode(WIFI_NONE_SLEEP);

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

    Serial.println("Initializing remote debug...");
    blinker.attach(0.3, blink);
    setClock();
    // remote debugging uses TimeLib so sync it
    setSyncProvider([] { return time(nullptr); });
    setupRemoteDebug(ssid);
    blinker.detach();

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
        blinker.attach(1, blink);
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
        blinker.detach();
        delay(500);
    }

    if (homekit->paired) {
        blinker.attach(0.33, blink);
        Serial.println("Connecting to heat pump... no more serial logging");
        MIE_LOG("Connecting to heat pump...");
        delay(200);
        if (setupHeatPump()) {
            blinker.detach();
            MIE_LOG("Heat pump connected");
        } else {
            blinker.attach(0.1, blink);

            Serial.begin(115200);
            Serial.println();
            Serial.println("Heat pump connection failed");
            MIE_LOG("Heat pump connection failed");
        }

        initHumidityReporting();
    }

    timer.once(2, stopBlinker);
}

void loop() {
    ArduinoOTA.handle();
    arduino_homekit_loop();
    Debug.handle();
}
