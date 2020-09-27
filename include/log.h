#ifndef LOG_H
#define LOG_H

#include <HardwareSerial.h>

#if MIEHK_DEBUG

#define DEBUG_LOG(...) do { \
	Serial.printf(__VA_ARGS__); \
} while(0)

#else

#define DEBUG_LOG(...)

#endif // DEBUG

#endif // LOG_H
