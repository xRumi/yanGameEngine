#pragma once
#include "defines.h"
#include "renderer_vulkan.h"
#include "platform.h"
#include <time.h>
#include "asset_manager.h"

typedef struct EngineState {
} EngineState;

void engineInitialization();
void engineRun();
void engineShutdown();
EngineState* engineGetEngineState();