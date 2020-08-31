#include <Arduino.h>

extern "C" size_t serial_print(const char s[]) {
    return Serial.print(s);
}

extern "C" size_t serial_println(const char s[]) {
    return Serial.println(s);
}

extern "C" size_t serial_print_int(int value) {
    Serial.print(value);
}

extern "C" size_t serial_print_float(float value, uint digits) {
    Serial.print(value, digits);
}
