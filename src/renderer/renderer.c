#include "renderer.h"
#include <threads.h>

void rendererInitialize() {
    thrd_t rendererThread;
    thrd_start_t rendererThreadEnter = vulkanRendererThreadEnter;
    thrd_create(&rendererThread, rendererThreadEnter, NULL);
}