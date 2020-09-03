#include <Arduino.h>

#include "log.h"

extern "C" void serial_print(const char s[]) {
    DEBUG_PRINT(s);
}

extern "C" void serial_println(const char s[]) {
    DEBUG_PRINTLN(s);
}

extern "C" void serial_print_int(int value) {
    DEBUG_PRINT(value);
}

extern "C" void serial_log_value(const char s[], int value) {
    DEBUG_PRINT(s);
    DEBUG_PRINTLN(value);
}
