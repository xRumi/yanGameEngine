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

    Model* boxModel = modelCreate("./assets/world/models/Cube", "Cube.gltf");
    Model* terrainModel = modelCreate("./assets/world/", "terrain.gltf");

    Scene* scene = sceneCreate();

    Entity* box1 = entityCreate(boxModel);
    Entity* box2 = entityCreate(boxModel);
    Entity* terrain = entityCreate(terrainModel);

    sceneAddEntity(scene, box1);
    sceneAddEntity(scene, box2);
    sceneAddEntity(scene, terrain);

    PointLight* light = sceneAddPointLight(scene);
    *light = (PointLight){
        .ambient = {{.1, .1, .1}},
        .position = {{0, 5, 0}},
        .diffuse = {{1, 1, 1}},
        .linear = 0.007,
        .quadratic = 0.00098
    };

    scene->camera.position = (vec3){{0, 0, 8}};

    rendererSceneSet(scene);

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
        entityApplyTransform(box1, mat4_translation(-1.5, 0, 0));

        entityResetTransform(box2);
        entityApplyTransform(box2, mat4_translation(1.5, 0, 0));
        entityApplyTransform(box2, mat4_view_YXZ((vec3){{0, 0, 0}}, (vec3){{fmod(elapsedTime, TO_RADIANS(360)), fmod(elapsedTime, TO_RADIANS(360)), 0}}));

        entityResetTransform(terrain);
        entityApplyTransform(terrain, mat4_translation(0, 0, 0));

        platformPullEvent();
        platformSleep(1.0 / 144.0);
    }

    engineShutdown();
}
