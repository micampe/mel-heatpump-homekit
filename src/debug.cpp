#include "debug.h"

#include <Ticker.h>
#include <stdio.h>
#include <xlogger.h>

#include "homekit.h"
#include "mqtt.h"
#include "settings.h"

xLogger Debug;

#define STATS_INTERVAL 11
static Ticker statsTicker;

static char *heapFreeTopic;
static char *heapMaxTopic;
static char *stackFreeTopic;
static char *homeKitClients;

void debug_init(const char ssid[]) {
#ifdef MIE_DEBUG
    Serial.println("Initializing remote debug...");

    Debug.begin(ssid);
    Debug.setProgramVersion((char *)GIT_DESCRIBE);
    Debug.setShowDebugLevel(false);
    Debug.setTimeFormat(ltUTCTime);
    Debug.setSerial(&Serial);
    Debug.enableSerial(true);
    MIE_LOG("%s remote log connected", ssid);
#endif

    if (settings.debug) {
        MIE_LOG("Memory stats reporting enabled");
        asprintf(&heapFreeTopic, "debug/%s/heap_free", ssid);
        asprintf(&heapMaxTopic, "debug/%s/heap_max", ssid);
        asprintf(&stackFreeTopic, "debug/%s/stack_free", ssid);
        asprintf(&homeKitClients, "debug/%s/homekit_clients", ssid);

        statsTicker.attach_scheduled(STATS_INTERVAL, [] {
            char str[6];
            snprintf(str, sizeof(str), "%u", ESP.getFreeHeap());
            mqtt.publish(heapFreeTopic, str);
            snprintf(str, sizeof(str), "%u", ESP.getMaxFreeBlockSize());
            mqtt.publish(heapMaxTopic, str);
            snprintf(str, sizeof(str), "%u", ESP.getFreeContStack());
            mqtt.publish(stackFreeTopic, str);
            // N * 1000 to scale it similar to memory values
            snprintf(str, sizeof(str), "%d", homekit_clients_count() * 1000);
            mqtt.publish(homeKitClients, str);
        });
    }
}

void logger_set_serial_enabled(bool enabled) {
    Debug.enableSerial(enabled);
}

void debug_loop() {
    Debug.handle();
}
