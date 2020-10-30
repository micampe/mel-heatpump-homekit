#include <Arduino.h>
#include <TimeLib.h>
#include <TZ.h>
#include <coredecls.h>

#include "ntp_clock.h"
#include "debug.h"


static bool timeWasSet = false;

void ntp_clock_init() {
    configTime(TZ_Etc_UTC, "pool.ntp.org");

    settimeofday_cb([] {
        timeWasSet = true;
        setTime(time(nullptr));
    });

    MIE_LOG("Syncing NTP time");
    int timeout = 4;
    int tick = 0;
    while (!timeWasSet && tick++ < timeout) {
        delay(500);
    }
    if (!timeWasSet) {
        MIE_LOG("NTP timed out");
    }

    setSyncProvider([] { return time(nullptr); });
}
