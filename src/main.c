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

    sceneCameraSetPosition(scene, (vec3){{0, 0, 8}});

    rendererSceneSet(scene);

    bool locked = false;
    PassiveDelay lKey = passiveDelaySet(0.3);
    PassiveDelay xKey = passiveDelaySet(0.3);
    PassiveDelay escKey = passiveDelaySet(0.3);

    PassiveDelay entityGenDelay = passiveDelaySet(.1);
    int objectCount = 0;

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

        if (passiveDelayIsDoneIfSoReset(&entityGenDelay) && objectCount < 100) {
            Entity* sphere = sceneCreateEntity(scene, sphereModel);
            entityTransformSetTranslation(sphere, (vec3){{-3, 3, 0}});
            sceneEntityApplyTransform(scene);
            sceneEntityCreatePhysicsBody(scene, sphere);
            physicsBodySetCollidable(sphere->physicsBody, true);
            entityPhysicsBodyAddForce(sphere, (vec3){{600}});
            entitySetHidden(sphere, false);
            objectCount++;
        }
        physicsEngineRun(scene->physicsEngine, 1.0 / 120.0);
        Entity* entity;
        hashmap_foreach(scene->entities, entity) {
            entity->transform.translation.x = clamp(entity->transform.translation.x, -3, 3);
            entity->transform.translation.y = MAX(entity->transform.translation.y, -3);
            if (entity->transform.translation.y == -3) entity->physicsBody->velocity.y = 0;
            if (entity->transform.translation.x == -3 || entity->transform.translation.x == 3) entity->physicsBody->velocity.x *= -1;
        }
        sceneEntityApplyTransform(scene);
        platformSleep(1.0 / fps);
    }
    engineShutdown();

    WARN("Object Count = %d", objectCount);
}
