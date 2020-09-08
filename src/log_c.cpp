#include "log.h"

extern "C" void debug_log(const char s[]) {
    DEBUG_LOG(s);
}

extern "C" void debug_log_value(const char s[], int value) {
    DEBUG_LOG(s, value);
}
