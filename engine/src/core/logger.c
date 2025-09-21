#include "logger.h"
#include <stdarg.h>
#include <stdio.h>

bool initializeLogger() {
    // TODO: create log file
    return true;
}

bool destroyLogger() {
    return true;
}

const char* levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};

void logOutput(logLevel level, const char* message, ...) {
    if (level < LOG_LEVEL_FATAL || level > LOG_LEVEL_TRACE) return;
    int messageLimit = 32768;
    char output[messageLimit + 16]; // 32k message limit;
    __builtin_va_list args;
    va_start(args, message);
    vsnprintf(output, messageLimit, message, args);
    va_end(args);
    // TODO: do something else based on config
    printf("%s%s\n", levelStrings[level], output);
}