#include <Arduino.h>

extern "C" void serial_print(const char s[]) {
    Serial.print(s);
}

extern "C" void serial_println(const char s[]) {
    Serial.println(s);
}

extern "C" void serial_print_int(int value) {
    Serial.print(value);
}

extern "C" void serial_print_float(float value, uint digits) {
    Serial.print(value, digits);
}

extern "C" void serial_log_value(const char s[], int value) {
    Serial.print(s);
    Serial.println(value);
}
