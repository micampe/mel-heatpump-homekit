#include "web.h"

#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <Time.h>
#include <arduino_homekit_server.h>
// this needs to be after the homekit header
#include <ArduinoJson.h>

#include "debug.h"
#include "heatpump_client.h"

// CLI update:
// curl -F "firmware=@<FILENAME>.bin" <ADDRESS>/_update

#define CONFIG_FILE "/config.json"

Settings settings;
#define JSON_CAPACITY JSON_OBJECT_SIZE(4) + sizeof(Settings)

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer updateServer;

using namespace mime;

extern const char* index_html;

void uptimeString(char* str, int size) {
    long val = millis() / 1000;
    int days = elapsedDays(val);
    int hours = numberOfHours(val);
    int minutes = numberOfMinutes(val);
    int seconds = numberOfSeconds(val);

    if (days > 0) {
        snprintf(str, size, "%dd %dh %dm %ds", days, hours, minutes, seconds);
    } else if (hours > 0) {
        snprintf(str, size, "%dh %dm %ds", hours, minutes, seconds);
    } else if (minutes > 0) {
        snprintf(str, size, "%dm %ds", minutes, seconds);
    } else {
        snprintf(str, size, "%ds", seconds);
    }
}

void loadSettings() {
    File config = LittleFS.open(CONFIG_FILE, "r");
    StaticJsonDocument<JSON_CAPACITY> doc;
    DeserializationError error = deserializeJson(doc, config);
    if (error) {
        MIE_LOG("Error loading coonfiguration file");
    } else {
        settings.mqtt_port = doc["mqtt_port"] | 1883;
        strlcpy(settings.mqtt_server, doc["mqtt_server"] | "", sizeof(settings.mqtt_server));
        strlcpy(settings.mqtt_temp, doc["mqtt_temp"] | "", sizeof(settings.mqtt_temp));
        strlcpy(settings.mqtt_humidity, doc["mqtt_hum"] | "", sizeof(settings.mqtt_humidity));
    }
    config.close();
}

void initSettings() {
    LittleFS.begin();
    if (!LittleFS.exists(CONFIG_FILE)) {
        File config = LittleFS.open(CONFIG_FILE, "w");
        config.write("{}");
        config.close();
    }
}

void initWeb(const char* hostname) {
    initSettings();
    loadSettings();

    updateServer.setup(&httpServer, "/_update");

    httpServer.on("/", HTTP_GET, []() {
        char heap[7];
        snprintf(heap, 7, "%d.%03d", ESP.getFreeHeap() / 1000, ESP.getFreeHeap() % 1000);
        char uptime[20];
        uptimeString(uptime, 20);

        String response = String(index_html);
        response.replace("__TITLE__", WiFi.hostname());
        response.replace("__HEAT_PUMP_STATUS__", heatpump.isConnected() ? "connected" : "not connected");
        response.replace("__UPTIME__", uptime);
        response.replace("__HEAP__", String(heap));
        response.replace("__FIRMWARE_VERSION__", GIT_DESCRIBE);

        httpServer.send(200, mimeTable[html].mimeType, response);
    });

    httpServer.on("/_settings", HTTP_GET, []() {
        File config = LittleFS.open(CONFIG_FILE, "r");
        char bytes[config.size() + 1];
        config.readBytes(bytes, config.size());
        bytes[config.size()] = '\0';
        httpServer.send(200, mimeTable[json].mimeType, bytes);
    });

    httpServer.on("/_settings", HTTP_POST, []() {
        File config = LittleFS.open(CONFIG_FILE, "r");
        StaticJsonDocument<JSON_CAPACITY> doc;
        deserializeJson(doc, config);
        config.close();

        for (uint8_t i = 0; i < httpServer.args(); i++) {
            String arg = httpServer.argName(i);
            String value = httpServer.arg(i);
            if (arg != "plain") {
                if (value.length() > 0) {
                    doc[arg] = value;
                } else {
                    doc.remove(arg);
                }
            }
        }

        config = LittleFS.open(CONFIG_FILE, "w");
        serializeJson(doc, config);

        size_t size = measureJson(doc);
        char response[size];
        serializeJsonPretty(doc, response, size);
        config.close();

        httpServer.send(200, mimeTable[json].mimeType, response);
        // delay(1000);
        // ESP.restart();
    });

    httpServer.on("/_reboot", HTTP_POST, []() {
        MIE_LOG("Reboot from web UI");
        httpServer.send(200, mimeTable[html].mimeType, "Rebooting...");
        delay(1000);
        ESP.restart();
    });

    httpServer.on("/_unpair", HTTP_POST, []() {
        MIE_LOG("Reset HomeKit pairing");
        httpServer.send(200, mimeTable[html].mimeType, "Reset HomeKit pairing. Rebooting...");
        homekit_storage_reset();
        delay(1000);
        ESP.restart();
    });

    MDNS.begin(hostname);
    MDNS.addService("http", "tcp", 80);
    httpServer.begin();
}
