#include "debug.h"

xLogger Debug;

const char* commandsDesc = "restart restarts ESP.";

bool handleCommands(String &cmd) {
    if (cmd == "restart") {
        MIE_LOG("restarting...");
        delay(1000);
        ESP.restart();
        return true;
    }

    return false;
}

void setupRemoteDebug(const char ssid[]) {
    Debug.begin(ssid);
    Debug.setProgramVersion((char *)GIT_DESCRIBE);
    Debug.setShowDebugLevel(false);
    Debug.setTimeFormat(ltUTCTime);
    Debug.cmdCallback(handleCommands, commandsDesc);
    MIE_LOG("%s connected", ssid);
}
