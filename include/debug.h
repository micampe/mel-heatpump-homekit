#ifndef MIE_DEBUG_H
#define MIE_DEBUG_H

#include <xlogger.h>

extern xLogger Debug;

void setupRemoteDebug(const char ssid[]);

#ifdef MIE_DEBUG
#define MIE_LOG(s, ...) Debug.printf(PSTR(s "\r\n"), ##__VA_ARGS__)
#else
#define MIE_LOG(...)
#endif

#endif
