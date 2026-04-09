#include "engine.h"
#include "emath.h"
#include "asset_types.h"
#include "asset_manager.h"
#include "rendererAPI.h"
#include "utils.h"

int main() {
    uint32_t width = 600, height = 600, fps = 144;
    engineInitialize("yanGameEngine - Triangle", 0, 0, width, height);
    rendererSetFPS(fps);

    Model* boxModel = modelCreate("./assets/world/models/Cube", "Cube.gltf");
    Model* terrainModel = modelCreate("./assets/world/", "terrain.gltf");

    Scene* scene = sceneCreate();

    Entity* box1 = entityCreate(boxModel);
    Entity* box2 = entityCreate(boxModel);
    Entity* terrain = entityCreate(terrainModel);
    
    entityTransformSetTranslation(box1, (vec3){{0, 3, -5}});
    entityTransformSetTranslation(box2, (vec3){{0, 0, -5}});

    sceneAddEntity(scene, box1);
    sceneAddEntity(scene, box2);
    sceneAddEntity(scene, terrain);

    sceneEntityPhysicsBodyCreate(scene, box1);
    sceneEntityPhysicsBodyCreate(scene, box2);
    physicsBodyStaticSet(box2->physicsBody, true);

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
    PassiveDelay lKey = passiveDelaySet(0.5);
    PassiveDelay xKey = passiveDelaySet(0.3);
    PassiveDelay escKey = passiveDelaySet(0.3);

    double startTime = platformGetTime();
    double deltaTime = 0;
    double currentTime, lastTime = platformGetTime();

    while (!platformGetPlatformState()->platformWindowClosed) {
        double elapsedTime = platformGetTime() - startTime;
        (void) elapsedTime, (void)deltaTime;
        currentTime = platformGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (platformInputIsKeyDown(KEY_l) && passiveDelayIsDoneThenReset(&lKey)) {
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
        if (platformInputIsKeyDown(KEY_x) && passiveDelayIsDoneThenReset(&xKey)) {
            rendererWireframeToggle();
        }
        if (platformInputIsKeyDown(KEY_ESC) && passiveDelayIsDoneThenReset(&escKey)) {
            platformGetPlatformState()->platformWindowClosed = true;
        }
    
        physicsEngineRun(scene->physicsEngine, 1.0 / 120.0);
        sceneEntityTransformApply(scene);

        platformPullEvent();
        platformSleep(1.0 / fps);
    }

    engineShutdown();
}
