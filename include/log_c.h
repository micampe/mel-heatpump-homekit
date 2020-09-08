#ifndef LOG_C_H
#define LOG_C_H

void debug_log(const char s[]);
void debug_log_value(const char s[], int value);

#ifdef MIEHK_DEBUG

#define DEBUG_LOG_C(s) debug_log(s "\n")
#define DEBUG_LOG_VALUE_C(s, value) debug_log_value(s " %d\n", (int)value)

#else

#define DEBUG_LOG_C(...)
#define DEBUG_LOG_VALUE_C(s, value)

#endif // MIEHK_DEBUG

#endif // LOG_C_H
