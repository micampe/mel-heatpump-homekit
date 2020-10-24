#include "debug.h"

#include <Ticker.h>
#include <stdio.h>

#include "mqtt.h"

xLogger Debug;

#define STATS_INTERVAL 11
Ticker statsTicker;

static char *heapFreeTopic;
static char *heapMaxTopic;
static char *stackFreeTopic;

void initRemoteDebug(const char ssid[]) {
#ifdef MIE_DEBUG
    Serial.println("Initializing remote debug...");

    Debug.begin(ssid);
    Debug.setProgramVersion((char *)GIT_DESCRIBE);
    Debug.setShowDebugLevel(false);
    Debug.setTimeFormat(ltUTCTime);
    MIE_LOG("%s connected", ssid);
#endif

    asprintf(&heapFreeTopic, "debug/%s/heap_free", ssid);
    asprintf(&heapMaxTopic, "debug/%s/heap_max", ssid);
    asprintf(&stackFreeTopic, "debug/%s/stack_free", ssid);

    statsTicker.attach(STATS_INTERVAL, [] {
        char str[6];
        snprintf(str, sizeof(str), "%u", ESP.getFreeHeap());
        mqtt.publish(heapFreeTopic, str);
        snprintf(str, sizeof(str), "%u", ESP.getMaxFreeBlockSize());
        mqtt.publish(heapMaxTopic, str);
        snprintf(str, sizeof(str), "%u", ESP.getFreeContStack());
        mqtt.publish(stackFreeTopic, str);
    });
}
