#include "logger.h"

bool loggerInitialize() {
    // TODO: create log file
    return true;
}

bool loggerDestroy() {
    return true;
}

const char* levelStrings[] = {
    ANSI_COLOR_RED ANSI_STYLE_BOLD "[FATAL] ",
    ANSI_COLOR_RED ANSI_STYLE_BOLD "[ERROR] ",
    ANSI_COLOR_RED ANSI_STYLE_BOLD "[WARN ] ",
    ANSI_COLOR_GREEN ANSI_STYLE_BOLD "[INFO ] " ANSI_RESET_ALL ANSI_COLOR_GREEN,
    ANSI_COLOR_MAGENTA ANSI_STYLE_BOLD "[DEBUG] " ANSI_RESET_ALL,
    ANSI_COLOR_BLUE ANSI_STYLE_BOLD "[TRACE] " ANSI_RESET_ALL,
    ANSI_COLOR_CYAN ANSI_STYLE_BOLD "[TEST ] " ANSI_RESET_ALL};

void logOutput(logLevel level, const char* message, ...) {
    bool skipLevelString = false;
    if (message[0] == '@') {
        skipLevelString = true;
        message++;
    }
    int messageLimit = 32768;
    char output[messageLimit + 16]; // 32k message limit;
    __builtin_va_list args;
    va_start(args, message);
    vsnprintf(output, messageLimit, message, args);
    va_end(args);
    // TODO: do something else based on config
    if (skipLevelString) printf("%s\n" ANSI_RESET_ALL, output);
    else printf("%s%s\n" ANSI_RESET_ALL, levelStrings[level], output);
}