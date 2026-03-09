#pragma once
#include "defines.h"
#include <stdarg.h>

#define ANSI_RESET_ALL          "\x1b[0m"

#define ANSI_COLOR_BLACK        "\x1b[30m"
#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[33m"
#define ANSI_COLOR_BLUE         "\x1b[34m"
#define ANSI_COLOR_MAGENTA      "\x1b[35m"
#define ANSI_COLOR_CYAN         "\x1b[36m"
#define ANSI_COLOR_WHITE        "\x1b[37m"

#define ANSI_BACKGROUND_BLACK   "\x1b[40m"
#define ANSI_BACKGROUND_RED     "\x1b[41m"
#define ANSI_BACKGROUND_GREEN   "\x1b[42m"
#define ANSI_BACKGROUND_YELLOW  "\x1b[43m"
#define ANSI_BACKGROUND_BLUE    "\x1b[44m"
#define ANSI_BACKGROUND_MAGENTA "\x1b[45m"
#define ANSI_BACKGROUND_CYAN    "\x1b[46m"
#define ANSI_BACKGROUND_WHITE   "\x1b[47m"

#define ANSI_STYLE_BOLD         "\x1b[1m"
#define ANSI_STYLE_ITALIC       "\x1b[3m"
#define ANSI_STYLE_UNDERLINE    "\x1b[4m"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1
#define LOG_TEST_ENABLED 1

#ifdef NDEBUG
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum logLevel {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_TEST
} logLevel;

void logOutput(logLevel level, const char* message, ...);

#define FATAL(message, ...) { logOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__); exit(0); }

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

#if LOG_TEST_ENABLED
#define TEST(message, ...) logOutput(LOG_LEVEL_TEST, message, ##__VA_ARGS__)
#else
#define TEST(message, ...)
#endif