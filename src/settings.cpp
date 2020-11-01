#include "settings.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

Settings settings;
#define JSON_CAPACITY 512

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
    const char *value = doc["debug"] | "0";
    settings.debug = strcmp(value, "1") == 0 || strcmp(value, "true") == 0;

    f.close();
}
