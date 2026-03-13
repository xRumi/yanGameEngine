#pragma once
#include "defines.h"
#include "platform.h"
#include <time.h>
#include "asset_manager.h"
#include "renderer.h"

typedef struct EngineState {
} EngineState;

void engineInitialize();
void engineLoop();
void engineShutdown();
EngineState* engineGetEngineState();