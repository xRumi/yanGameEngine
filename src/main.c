#include "engine.h"
#include "emath.h"
#include "asset_types.h"
#include "asset_manager.h"
#include "rendererAPI.h"
#include "utils.h"

int main() {
    uint32_t width = 600, height = 600;
    engineInitialize("yanGameEngine - Triangle", 0, 0, width, height);
    rendererSetFPS(144);

    Model* boxModel = modelCreate("./assets/world/models/BoxTextured", "BoxTextured.gltf");
    Model* terrainModel = modelCreate("./assets/world/", "terrain.gltf");

    Entity* box1 = entityCreate(boxModel);
    Entity* box2 = entityCreate(boxModel);
    Entity* terrain = entityCreate(terrainModel);

    rendererAddEntity(box1);
    rendererAddEntity(box2);
    rendererAddEntity(terrain);

    rendererCameraSetPosition((vec3){{0, -6, 0}});

    bool locked = false;
    PassiveDelay lockKey = passiveDelaySet(0.5);
    PassiveDelay xKey = passiveDelaySet(0.3);

    double startTime = platformGetTime();
    while (!platformGetPlatformState()->platformWindowClosed) {
        double elapsedTime = platformGetTime() - startTime;
        (void) elapsedTime;

        if (platformInputIsKeyDown(KEY_l) && passiveDelayIsDone(lockKey)) {
            if (!locked) {
                platformPointerHide();
                platformPointerLock();
                locked = true;
            } else {
                platformPointerUnhide();
                platformPointerUnlock();
                locked = false;
            }
            passiveDelayReset(&lockKey);
        }
        if (platformInputIsKeyDown(KEY_x) && passiveDelayIsDone(xKey)) {
            rendererWireframeToggle();
            passiveDelayReset(&xKey);
        }

        entityResetTransform(box1);
        entityApplyTransform(box1, mat4_translation(-1, 0, 0));
        entityApplyTransform(box1, mat4_mul(mat4_rotation_x(35 * elapsedTime), mat4_rotation_y(35 * elapsedTime)));

        entityResetTransform(box2);
        entityApplyTransform(box2, mat4_translation(1, 0, 0));
        entityApplyTransform(box2, mat4_mul(mat4_rotation_x(85 * elapsedTime), mat4_rotation_y(85 * elapsedTime)));

        entityResetTransform(terrain);
        entityApplyTransform(terrain, mat4_translation(0, 0, 0));


        platformPullEvent();
        platformSleep(1.0 / 144.0);
    }

    engineShutdown();
}
