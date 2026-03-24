#pragma once
#include "defines.h"

typedef struct PlatformState {
    void* display;
    void* surface;
    uint32_t width;
    uint32_t height;
    bool platformWindowClosed;
} PlatformState;

void platformInitialize(const char* windowTitle, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void platformPullEvent();
void platformShutdown();

PlatformState* platformGetPlatformState();

void platformConsoleWrite(const char* message, uint8_t color);
double platformGetTime();
void platformSleep(double time);

uint64_t platformThreadCreate(void*(*fun)(void*), void* arg);