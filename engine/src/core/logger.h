#pragma once
#include "defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#ifdef NDEBUG
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum logLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} logLevel;

bool initializeLogger();
bool destroyLogger();
void logOutput(logLevel level, const char* message, ...);

#define FATAL(message, ...) logOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)
#define ERROR(message, ...) logOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)

#if LOG_WARN_ENABLED
#define WARN(message, ...) logOutput(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#else
#define WARN(message, ...)
#endif

#if LOG_INFO_ENABLED
#define INFO(message, ...) logOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#else
#define INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED
#define DEBUG(message, ...) logOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#else
#define DEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED
#define TRACE(message, ...) logOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#else
#define TRACE(message, ...)
#endif