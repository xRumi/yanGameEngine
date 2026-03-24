#include "engine.h"

EngineState* engineState;

volatile int volatile_true  = 1;
volatile int volatile_false = 0;

void commonAssets() {
    assert(sizeof(char) == 1);
    assert(sizeof(vec2) == 8);
    assert(sizeof(vec3) == 12);
}

void engineInitialize(const char* windowTitle) {
    commonAssets();

    srand(time(NULL));
    
    engineState = memalloc(sizeof(EngineState), MEMORY_TAG_ENGINE);
    platformInitialize(windowTitle, 0, 0, 600, 600);
    rendererInitialize();
}

void engineShutdown() {
    INFO("Memory leaked:\n%s", get_memory_usage_str());
    platformShutdown();
}

EngineState* engineGetEngineState() {
    return engineState;
}
