#include <Arduino.h>
#include <TelnetStream.h>

#define DEBUG_PRINT(value, ...) do { \
	Serial.print(value, ##__VA_ARGS__); \
	TelnetStream.print(value, ##__VA_ARGS__); \
} while(0)

#define DEBUG_PRINTLN(value, ...) do { \
	Serial.println(value, ##__VA_ARGS__); \
	TelnetStream.println(value, ##__VA_ARGS__); \
} while(0)
