#include <Arduino.h>
#include <TimeLib.h>
#include <TZ.h>
#include <coredecls.h>

#include "ntp_clock.h"


static bool timeWasSet = false;

void ntp_clock_init() {
    configTime(TZ_Etc_UTC, "pool.ntp.org");

    settimeofday_cb([] {
        timeWasSet = true;
    });

    Serial.print("Syncing NTP time...");
    int timeout = 4;
    int tick = 0;
    while (!timeWasSet && tick++ < timeout) {
        delay(500);
        Serial.print('.');
    }
    if (!timeWasSet) {
        Serial.println(" timed out");
    } else {
        Serial.println();
    }

    setSyncProvider([] { return time(nullptr); });
}
