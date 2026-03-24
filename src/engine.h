#pragma once
#include "defines.h"
#include "platform.h"
#include <time.h>
#include "asset_manager.h"
#include "renderer.h"

typedef struct EngineState {
} EngineState;

void engineInitialize(const char* windowTitle);
void engineShutdown();
EngineState* engineGetEngineState();