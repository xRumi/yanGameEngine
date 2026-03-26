#include "engine.h"

EngineState* engineState;

volatile int volatile_true  = 1;
volatile int volatile_false = 0;

void commonAssets() {
    assert(sizeof(char) == 1);
    assert(sizeof(vec2) == 8);
    assert(sizeof(vec3) == 12);
}

void engineInitialize(const char *windowTitle, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    commonAssets();

    srand(time(NULL));
    
    engineState = memalloc(sizeof(EngineState), MEMORY_TAG_ENGINE);
    platformInitialize(windowTitle, x, y, width, height);
    rendererInitialize();
}

void engineShutdown() {
    INFO("Memory leaked:\n%s", get_memory_usage_str());
    platformShutdown();
}

EngineState* engineGetEngineState() {
    return engineState;
}
