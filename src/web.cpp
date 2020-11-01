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
#include "mqtt.h"
#include "wifi_manager.h"

// CLI update:
// curl -F "firmware=@<FILENAME>.bin" <ADDRESS>/_update

#define CONFIG_FILE "/config.json"

Settings settings;
#define JSON_CAPACITY 512

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer updateServer;

using namespace mime;

extern const char* index_html;

template <size_t SIZE>
static void status_heap(char (&str)[SIZE]) {
    snprintf(str, sizeof(str), "%d.%03dB / %d%% / %d.%03dB",
            ESP.getFreeHeap() / 1000, ESP.getFreeHeap() % 1000,
            ESP.getHeapFragmentation(),
            ESP.getMaxFreeBlockSize() / 1000, ESP.getMaxFreeBlockSize() % 1000);
}

static void status_uptime(char (&str)[20]) {
    long val = millis() / 1000;
    int days = elapsedDays(val);
    int hours = numberOfHours(val);
    int minutes = numberOfMinutes(val);
    int seconds = numberOfSeconds(val);

    size_t size = sizeof(str);
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

static void status_mqtt(char (&str)[22]) {
    size_t size = sizeof(str);
    if (!mqtt_is_configured()) {
        strlcpy(str, "not configured", size);
    } else if (mqtt.state() == MQTT_CONNECTED) {
        strlcpy(str, "connected", size);
    } else if (mqtt.state() == MQTT_DISCONNECTED) {
        strlcpy(str, "disconnected", size);
    } else {
        snprintf(str, size, "connection error: %d", mqtt.state());
    }
}

static void status_homekit(char (&str)[20]) {
    size_t size = sizeof(str);
    homekit_server_t *homekit = arduino_homekit_get_running_server();
    if (homekit->paired) {
        int clients = arduino_homekit_connected_clients_count();
        snprintf(str, size, "paired, %d client%s", clients, clients == 1 ? "" : "s");
    } else {
        snprintf(str, size, "waiting for pairing");
    }
}

void settings_init() {
    LittleFS.begin();
    File f = LittleFS.open(CONFIG_FILE, "r");
    if (!f) {
        MIE_LOG("Creating empty config file");
        f = LittleFS.open(CONFIG_FILE, "w");
        f.write("{}");
        f.close();
        f = LittleFS.open(CONFIG_FILE, "r");
    }

    StaticJsonDocument<JSON_CAPACITY> doc;
    DeserializationError error = deserializeJson(doc, f);
    if (error) {
        MIE_LOG("Error loading configuration file");
        LittleFS.remove(CONFIG_FILE);
    }

    uint16_t port = strtol(doc["mqtt_port"] | "", nullptr, 10);
    settings.mqtt_port = port > 0 ? port : 1883;
    strlcpy(settings.mqtt_server, doc["mqtt_server"] | "", sizeof(settings.mqtt_server));
    strlcpy(settings.mqtt_temp, doc["mqtt_temp"] | "", sizeof(settings.mqtt_temp));
    strlcpy(settings.mqtt_humidity, doc["mqtt_hum"] | "", sizeof(settings.mqtt_humidity));
    strlcpy(settings.mqtt_dew_point, doc["mqtt_dew_point"] | "", sizeof(settings.mqtt_dew_point));

    f.close();
}

static void web_get_settings() {
    File config = LittleFS.open(CONFIG_FILE, "r");
    size_t content_size = config.size();
    char bytes[content_size + 1];
    config.readBytes(bytes, content_size);
    bytes[content_size] = '\0';
    httpServer.send(200, mimeTable[json].mimeType, bytes);
}

static void web_post_settings() {
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
    serializeJsonPretty(doc, config);

    size_t size = measureJsonPretty(doc);
    char response[size + 1];
    serializeJsonPretty(doc, response, sizeof(response));
    config.close();

    httpServer.send(200, mimeTable[json].mimeType, response);
    delay(1000);
    ESP.restart();
}

static void web_get_status() {
    char heap_str[25];
    status_heap(heap_str);
    char uptime_str[20];
    status_uptime(uptime_str);
    char mqtt_str[22];
    status_mqtt(mqtt_str);
    char homekit_str[20];
    status_homekit(homekit_str);
    char firmware_str[30];
    snprintf(firmware_str, sizeof(firmware_str), "%s (%s)", GIT_DESCRIBE, GIT_HASH);

    StaticJsonDocument<JSON_CAPACITY> doc;
    doc["title"] = WiFi.hostname();
    doc["heatpump"] = heatpump.isConnected() ? "connected" : "not connected";
    doc["homekit"] = homekit_str;
    doc["env"] = strlen(env_sensor_status) ? env_sensor_status : "not connected";
    doc["mqtt"] = mqtt_str;
    doc["uptime"] = uptime_str;
    doc["heap"] = heap_str;
    doc["firmware"] = firmware_str;

    size_t doc_size = measureJsonPretty(doc);
    char response[doc_size + 1];
    serializeJsonPretty(doc, response, sizeof(response));
    httpServer.send(200, mimeTable[json].mimeType, response);
}

static void web_post_reboot() {
    MIE_LOG("Reboot from web UI");
    httpServer.send(200, mimeTable[html].mimeType, "Rebooting...");
    delay(1000);
    ESP.restart();
}

static void web_post_reset_wifi() {
    MIE_LOG("Reset WiFi settings");
    httpServer.send(200, mimeTable[html].mimeType, "Reset WiFi settings. Rebooting...");
    delay(1000);
    wifiManager.resetSettings();
    ESP.restart();
}

static void web_post_unpair() {
    MIE_LOG("Reset HomeKit pairing");
    httpServer.send(200, mimeTable[html].mimeType, "Reset HomeKit pairing. Rebooting...");
    delay(1000);
    homekit_storage_reset();
    ESP.restart();
}

void web_init(const char* hostname) {
    settings_init();
    updateServer.setup(&httpServer, "/_update");

    httpServer.on("/", HTTP_GET, []() {
        httpServer.send(200, mimeTable[html].mimeType, index_html);
    });

    httpServer.on("/_settings", HTTP_GET, web_get_settings);
    httpServer.on("/_settings", HTTP_POST, web_post_settings);
    httpServer.on("/_status", HTTP_GET, web_get_status);
    httpServer.on("/_reboot", HTTP_POST, web_post_reboot);
    httpServer.on("/_reset_wifi", HTTP_POST, web_post_reset_wifi);
    httpServer.on("/_unpair", HTTP_POST, web_post_unpair);

    MDNS.begin(hostname);
    MDNS.addService("http", "tcp", 80);
    httpServer.begin();
}
