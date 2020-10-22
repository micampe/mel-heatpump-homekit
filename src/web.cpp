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
#include "env_sensor.h"
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

static void heapStatus(char* str, size_t size) {
    snprintf(str, size, "%d.%03dB / %d%%", ESP.getFreeHeap() / 1000, ESP.getFreeHeap() % 1000, ESP.getHeapFragmentation());
}

static void uptimeString(char* str, int size) {
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

static void mqttStatus(char* str, size_t size) {
    if (!mqttIsConfigured()) {
        strlcpy(str, "not configured", size);
    } else if (mqtt.lastError() == LWMQTT_SUCCESS) {
        if (strlen(env_sensor_status) > 0) {
            strlcpy(str, "connected", size);
        } else {
            strlcpy(str, "not connected", size);
        }
    } else {
        snprintf(str, size, "connection error: %d", mqtt.lastError());
    }
}

static void homeKitStatus(char* str, size_t size) {
    homekit_server_t *homekit = arduino_homekit_get_running_server();
    if (homekit->paired) {
        int clients = arduino_homekit_connected_clients_count();
        snprintf(str, size, "paired, %d client%s", clients, clients == 1 ? "" : "s");
    } else {
        snprintf(str, size, "waiting for pairing");
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
        strlcpy(settings.mqtt_dew_point, doc["mqtt_dew_point"] | "", sizeof(settings.mqtt_dew_point));
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
        httpServer.send(200, mimeTable[html].mimeType, index_html);
    });

    httpServer.on("/_settings", HTTP_GET, []() {
        File config = LittleFS.open(CONFIG_FILE, "r");
        size_t content_size = config.size();
        char bytes[content_size];
        config.readBytes(bytes, content_size);
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
        delay(1000);
        ESP.restart();
    });

    httpServer.on("/_status", HTTP_GET, []() {
        char heap_status[15];
        heapStatus(heap_status, sizeof(heap_status));
        char uptime[20];
        uptimeString(uptime, sizeof(uptime));
        char mqtt_status[30];
        mqttStatus(mqtt_status, sizeof(mqtt_status));
        char homekit_status[20];
        homeKitStatus(homekit_status, sizeof(homekit_status));

        StaticJsonDocument<500> doc;
        doc["title"] = WiFi.hostname();
        doc["heatpump"] = heatpump.isConnected() ? "connected" : "not connected";
        doc["homekit"] = homekit_status;
        doc["env"] = strlen(env_sensor_status) ? env_sensor_status : "not connected";
        doc["mqtt"] = mqtt_status;
        doc["uptime"] = uptime;
        doc["heap"] = heap_status;
        doc["firmware"] = GIT_DESCRIBE;

        size_t doc_size = measureJson(doc);
        char response[doc_size + 1];
        serializeJson(doc, response, sizeof(response));
        httpServer.send(200, mimeTable[json].mimeType, response);
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
