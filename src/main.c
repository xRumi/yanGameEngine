#include "main.h"
#include "emath.h"
#include "asset_types.h"
#include "model_loader.h"
#include "rendererAPI.h"
#include <unistd.h>

int main() {
    engineInitialize();

    Model* boxModel = loadModel("./assets/world/models/BoxVertexColors/BoxVertexColors.gltf");
    Entity box = createEntity(boxModel);
    rendererDrawEntity(&box);

    engineLoop();
    engineShutdown();
}
