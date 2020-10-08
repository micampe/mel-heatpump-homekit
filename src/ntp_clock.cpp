#include <Arduino.h>
#include <TimeLib.h>
#include <TZ.h>
#include <coredecls.h>

#include "debug.h"
#include "ntp_clock.h"


static bool timeWasSet = false;

void initNTPClock() {
    configTime(TZ_Etc_UTC, "pool.ntp.org");

    settimeofday_cb([] {
        timeWasSet = true;
        MIE_LOG("NTP synced");
    });

    MIE_LOG("Syncing NTP time...");
    Serial.print("Syncing NTP time...");
    while (!timeWasSet) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    setSyncProvider([] { return time(nullptr); });
}
