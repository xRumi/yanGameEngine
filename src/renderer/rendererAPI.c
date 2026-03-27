#include "rendererAPI.h"
#include "renderer.h"

extern RendererState internalStateRenderer;

void rendererSetFPS(double fps) {
    internalStateRenderer.targetFrameTime = 1 / (double)fps;
}
void rendererCameraReset() {
    internalStateRenderer.camera = (Camera){
        .position = {{0, 0, 0}},
        .direction = {{0, 0, -1}},
        .up = {{0, 1, 0}},
        .dirty = true
    };
}
void rendererCameraSetPosition(vec3 position) {
    internalStateRenderer.camera.position.x = position.x;
    internalStateRenderer.camera.position.y = position.z;
    internalStateRenderer.camera.position.z = -position.y;
}
void rendererAddEntity(Entity* entity) {
    hashmap_put(internalStateRenderer.entities, entity->id, (uint64_t)entity);
}