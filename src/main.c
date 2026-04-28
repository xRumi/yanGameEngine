#include "engine.h"
#include "asset_types.h"
#include "asset_manager.h"
#include "rendererAPI.h"
#include "utils.h"

int main() {
    uint32_t width = 600, height = 600, fps = 144;
    engineInitialize("yanGameEngine - Physics Engine Test", 0, 0, width, height);
    rendererSetFPS(fps);

    Model* model = assetLoadGLTF("./assets/world/models/BoxAnimated", "BoxAnimated.gltf");
    Scene* scene = sceneCreate();

    Entity* entity = sceneCreateEntity(scene, model);
    entityTransformSetTranslation(entity, (vec3){{0, 0, 0}});
    sceneEntityApplyTransform(scene);
    entitySetHidden(entity, false);

    sceneAddDirectionalLight(scene)->ambient = (vec4){{1, 1, 1, 1}};
    sceneCameraSetPosition(scene, (vec3){{0, 0, 8}});
    rendererSetScene(scene);

    bool locked = false;
    PassiveDelay lKey = passiveDelaySet(0.3);
    PassiveDelay xKey = passiveDelaySet(0.3);
    PassiveDelay escKey = passiveDelaySet(0.3);

    while (!platformGetPlatformState()->isWindowClosed) {
        if (platformInputIsKeyDown(KEY_l) && passiveDelayIsDoneIfSoReset(&lKey)) {
            if (!locked) {
                platformPointerHide();
                platformPointerLock();
                locked = true;
            } else {
                platformPointerUnhide();
                platformPointerUnlock();
                locked = false;
            }
        }
        if (platformInputIsKeyDown(KEY_x) && passiveDelayIsDoneIfSoReset(&xKey)) {
            rendererWireframeToggle();
        }
        if (platformInputIsKeyDown(KEY_ESC) && passiveDelayIsDoneIfSoReset(&escKey)) {
            platformGetPlatformState()->isWindowClosed = true;
        }

        physicsEngineRun(scene->physicsEngine, 1.0 / 120.0);
        sceneEntityApplyTransform(scene);
        platformSleep(1.0 / fps);
    }
    engineShutdown();
}
