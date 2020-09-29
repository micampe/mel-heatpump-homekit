#include "debug.h"

xLogger Debug;

void setupRemoteDebug(const char ssid[]) {
    Debug.begin(ssid);
    Debug.setProgramVersion((char *)GIT_DESCRIBE);
    Debug.setShowDebugLevel(false);
    Debug.setTimeFormat(ltNone);
    MIE_LOG("%s connected", ssid);
}
