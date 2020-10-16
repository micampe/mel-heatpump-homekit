#include "wifi_manager.h"

#include <ArduinoOTA.h>
#include <DoubleResetDetect.h>
#include <WiFiManager.h>
#include <Ticker.h>
#include <arduino_homekit_server.h>

#include "led_status_patterns.h"

WiFiManager wifiManager;

#define DRD_TIMEOUT 2.0
#define DRD_ADDRESS 0x00
static DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

static WiFiManagerParameter mqttServer;
static WiFiManagerParameter mqttPort;
static WiFiManagerParameter publishTemperatureTopic;
static WiFiManagerParameter publishHumidityTopic;
static WiFiManagerParameter footerHtmlParam;

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

static void saveSettings() {
}

void initWiFiManager(const char* ssid) {
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    wifiManager.setAPCallback(wifiConfigModeCallback);
    wifiManager.setTimeout(120);
    wifiManager.setShowBackButton(true);

    new (&mqttServer) WiFiManagerParameter("mqttServer", "MQTT broker", "", 40, "placeholder='address'");
    new (&mqttPort) WiFiManagerParameter("mqttPort", "", "1883", 6, "placeholder='port'", WFM_NO_LABEL);
    new (&publishTemperatureTopic) WiFiManagerParameter("publishTemperatureTopic", "Environment sensor topics", "", 80, "placeholder='Temperature'");
    new (&publishHumidityTopic) WiFiManagerParameter("publishHumidityTopic", "", "", 80, "placeholder='Relative humidity'", WFM_NO_LABEL);

    wifiManager.addParameter(&mqttServer);
    wifiManager.addParameter(&mqttPort);
    wifiManager.addParameter(&publishTemperatureTopic);
    wifiManager.addParameter(&publishHumidityTopic);
    wifiManager.setSaveParamsCallback(saveSettings);

    const char* footerHtml = 
        "<p style='font-size:small;color:darkgray'>If these topics are set, temperature and relative humidity readings will be periodically posted to mqtt.<br><br>"
        "This feature requires a BME280 or DHT22 temperature and humidity sensor connected to the ESP8266.</p>";
    new (&footerHtmlParam) WiFiManagerParameter(footerHtml);
    wifiManager.addParameter(&footerHtmlParam);

    std::vector<const char*> menu = {"wifi", "sep", "info", "restart"};
    wifiManager.setMenu(menu);

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
