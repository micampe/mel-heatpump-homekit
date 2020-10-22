#include "debug.h"

xLogger Debug;

void initRemoteDebug(const char ssid[]) {
#ifdef MIE_DEBUG
    Serial.println("Initializing remote debug...");

    Debug.begin(ssid);
    Debug.setProgramVersion((char *)GIT_DESCRIBE);
    Debug.setShowDebugLevel(false);
    Debug.setTimeFormat(ltUTCTime);
    MIE_LOG("%s connected", ssid);
#endif
}
