#include "engine.h"
#include <threads.h>

EngineState* engineState;
extern int platformWindowClosed;

void engineInitialization() {
    assert(sizeof(char) == 1);
    assert(sizeof(vec2) == 8);
    assert(sizeof(vec3) == 12);

    srand(time(NULL));
    
    engineState = memalloc(sizeof(EngineState), MEMORY_TAG_ENGINE);
    platformInitialize("yanGameEngine", 0, 0, 600, 600);
    loggerInitialize();
    rendererInitialize();
    // TRACE("Total heap memory usage\n%s", get_memory_usage_str());
}

void engineRun() {
    while (!platformWindowClosed) {
        platformPullEvent();
        // just sleep
        platform_sleep(1.0 / 60.0);
    }
}

void engineShutdown() {
    platformShutdown();
}

EngineState* engineGetEngineState() {
    return engineState;
}
