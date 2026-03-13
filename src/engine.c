#include "engine.h"

EngineState* engineState;

void commonAssets() {
    assert(sizeof(char) == 1);
    assert(sizeof(vec2) == 8);
    assert(sizeof(vec3) == 12);
}

void engineInitialize() {
    commonAssets();

    srand(time(NULL));
    
    engineState = memalloc(sizeof(EngineState), MEMORY_TAG_ENGINE);
    platformInitialize("yanGameEngine", 0, 0, 600, 600);
    rendererInitialize();
}

void engineLoop() {
    while (!platformGetPlatformState()->platformWindowClosed) {
        platformPullEvent();
        platformSleep(1.0 / 60.0);
    }
}

void engineShutdown() {
    INFO("Memory leaked:\n%s", get_memory_usage_str());
    platformShutdown();
}

EngineState* engineGetEngineState() {
    return engineState;
}
