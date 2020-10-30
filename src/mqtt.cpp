#include "mqtt.h"

#include <Ticker.h>

#include "debug.h"
#include "web.h"

static WiFiClient net;
PubSubClient mqtt(net);

static const char *client_id;

bool mqtt_is_configured() {
    return strlen(settings.mqtt_server);
}

bool mqtt_init(const char* name) {
    if (mqtt_is_configured()) {
        client_id = name;
        MIE_LOG("Connecting to MQTT broker %s:%u", settings.mqtt_server, settings.mqtt_port);
        mqtt.setServer(settings.mqtt_server, settings.mqtt_port);
        return mqtt_connect();
    } else {
        MIE_LOG("MQTT reporting not configured");
        return false;
    }
}

bool mqtt_connect() {
    if (!mqtt_is_configured()) {
        return false;
    }

    if (mqtt.connected()) {
        return true;
    }

    const int timeout = 3;
    int tick = 0;
    while (!mqtt.connect(client_id) && tick++ < timeout) {
        delay(10);
    }

    if (tick < timeout) {
        return true;
    } else {
        MIE_LOG("MQTT connection failed");
        return false;
    }
}
