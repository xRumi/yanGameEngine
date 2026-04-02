#include "rendererAPI.h"
#include "renderer.h"

extern RendererState internalStateRenderer;

void rendererSetFPS(double fps) {
    internalStateRenderer.targetFrameTime = 1 / (double)fps;
}
void rendererSceneSet(Scene* scene) {
    internalStateRenderer.scene = scene;
}

void rendererEnableWireframe() {
    internalStateRenderer.useWireframe = true;
}
void rendererDisableWireframe() {
    internalStateRenderer.useWireframe = false;
}
void rendererWireframeToggle() {
    internalStateRenderer.useWireframe ^= true;
}