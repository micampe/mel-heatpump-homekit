#include "debug.h"

xLogger Debug;

void setupRemoteDebug(const char ssid[]) {
    Debug.begin(ssid);
    Debug.setProgramVersion((char *)GIT_DESCRIBE);
    MIE_LOG("%s connected", ssid);
}
