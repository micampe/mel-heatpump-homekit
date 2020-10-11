#include "wifi_manager.h"

#include <ArduinoOTA.h>
#include <DoubleResetDetect.h>
#include <WiFiManager.h>
#include <arduino_homekit_server.h>
#include <led_status.h>

#include "led_status_patterns.h"


#define DRD_TIMEOUT 2.0
#define DRD_ADDRESS 0x00
static DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);


static void wifiConnectionFailed() {
    led_status_signal(&status_led_error);
    Serial.println("WiFi connection failed, restarting");
    delay(1000);
    ESP.reset();
    delay(3000);
}

static void wifiConfigModeCallback(WiFiManager *wifiManager) {
    led_status_set(&status_led_waiting_wifi);
}

void initWiFiManager(const char* ssid) {
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    WiFiManager wifiManager;
    wifiManager.setAPCallback(wifiConfigModeCallback);
    wifiManager.setTimeout(120);

    if (drd.detect()) {
        Serial.println("Double reset detected");
        led_status_signal(&status_led_double_reset);

        // Serial.println("Clearing HomeKit config");
        // homekit_storage_reset();

        Serial.println("Starting config portal");
        if (!wifiManager.startConfigPortal(ssid)) {
            wifiConnectionFailed();
        }
    } else {
        if (!wifiManager.autoConnect(ssid)) {
            wifiConnectionFailed();
        }
    }
}
