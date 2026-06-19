#pragma once
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,
} android_LogPriority;

int __android_log_print(int prio, const char* tag, const char* fmt, ...)
    __attribute__((format(printf, 3, 4)));
int __android_log_vprint(int prio, const char* tag, const char* fmt, va_list ap);
void __android_log_write(int prio, const char* tag, const char* text);
void __android_log_assert(const char* cond, const char* tag, const char* fmt, ...)
    __attribute__((noreturn)) __attribute__((format(printf, 3, 4)));
int __android_log_is_loggable(int prio, const char* tag, int default_prio);

#ifdef __cplusplus
}
#endif
