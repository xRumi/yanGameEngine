#include "engine.h"
#include "asset_types.h"
#include "asset_manager.h"
#include "rendererAPI.h"
#include "utils.h"

int main() {
    uint32_t width = 600, height = 600, fps = 144;
    engineInitialize("yanGameEngine - Physics Engine Test", 0, 0, width, height);
    rendererSetFPS(fps);

    Model* sphereModel = assetGenerateUVSphere(8, 8, 0.08);
    Scene* scene = sceneCreate();

    sceneAddDirectionalLight(scene)->ambient = (vec4){{1, 1, 1, 1}};

    scene->camera.position = (vec3){{0, 0, 8}};

    rendererSceneSet(scene);

    bool locked = false;
    PassiveDelay lKey = passiveDelaySet(0.3);
    PassiveDelay xKey = passiveDelaySet(0.3);
    PassiveDelay escKey = passiveDelaySet(0.3);

    PassiveDelay entityGenDelay = passiveDelaySet(.1);

    double startTime = platformGetTime();
    double deltaTime = 0;
    double currentTime, lastTime = platformGetTime();

    while (!platformGetPlatformState()->platformWindowClosed) {
        double elapsedTime = platformGetTime() - startTime;
        (void) elapsedTime, (void)deltaTime;
        currentTime = platformGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

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
            platformGetPlatformState()->platformWindowClosed = true;
        }

        if (passiveDelayIsDoneIfSoReset(&entityGenDelay)) {
            Entity* sphere = sceneCreateEntity(scene, sphereModel);
            entityTransformSetTranslation(sphere, (vec3){{0, 3, 0}});
            sceneEntityApplyTransform(scene);
            sceneEntityCreatePhysicsBody(scene, sphere);
        }
    
        physicsEngineRun(scene->physicsEngine, 1.0 / 120.0);
        Entity* entity;
        hashmap_foreach(scene->entities, entity) {
            entity->transform.translation.x = clamp(entity->transform.translation.x, -3, 3);
            entity->transform.translation.y = clamp(entity->transform.translation.y, -3, 3);
            entity->physicsBody->forceAccumulator.x = (rand() % 2 ? 1 : -1)*(rand() % 20);
            entity->physicsBody->forceAccumulator.y = rand() % 20;
        }
        sceneEntityApplyTransform(scene);

        platformPullEvent();
        platformSleep(1.0 / fps);
    }

    engineShutdown();
}
