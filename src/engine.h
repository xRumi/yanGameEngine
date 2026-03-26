#pragma once
#include "defines.h"
#include "platform.h"
#include <time.h>
#include "asset_manager.h"
#include "renderer.h"

typedef struct EngineState {
} EngineState;

void engineInitialize(const char *windowTitle, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void engineShutdown();
EngineState* engineGetEngineState();