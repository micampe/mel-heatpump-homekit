#pragma once

#include <xlogger.h>

extern xLogger Debug;

void debug_init(const char ssid[]);
void debug_loop();
void logger_set_serial_enabled(bool enabled);

#ifdef MIE_DEBUG
#define MIE_LOG(s, ...) Debug.printf(PSTR(s "\r\n"), ##__VA_ARGS__)
#else
#define MIE_LOG(...)
#endif
