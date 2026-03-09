#include "engine.h"

EngineState* engineState;
extern int platformWindowClosed;

void commonAssets() {
    assert(sizeof(char) == 1);
    assert(sizeof(vec2) == 8);
    assert(sizeof(vec3) == 12);
}

void engineInitialization() {
    commonAssets();

    srand(time(NULL));
    
    engineState = memalloc(sizeof(EngineState), MEMORY_TAG_ENGINE);
    assetManagerInitialize();
    platformInitialize("yanGameEngine", 0, 0, 600, 600);
    rendererInitialize();
}

void engineRun() {
    while (!platformWindowClosed) {
        platformPullEvent();
        // just sleep
        platformSleep(1.0 / 60.0);
    }
}

void engineShutdown() {
    INFO("Memory leaked:\n%s", get_memory_usage_str());
    platformShutdown();
    assetManagerShutdown();
}

EngineState* engineGetEngineState() {
    return engineState;
}
