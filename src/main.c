#include "engine.h"
#include "asset_types.h"
#include "asset_manager.h"
#include "rendererAPI.h"
#include "utils.h"

void handleCamera(Camera* camera, double deltaTime) {
    vec3 cameraPosition = atomicVec3GetVec3(&camera->position);
    vec3 cameraRotation = atomicVec3GetVec3(&camera->rotation);

    if (platformWindowIsFocused() && platformPointerIsLocked()) {
        vec2 pointerRelative = platformInputPointerRelative();
        // rotation(euler angle): x = yaw, y = pitch, z = roll
        cameraRotation.x += -pointerRelative.x / (float)platformGetPlatformState()->width * camera->sensitivity;
        cameraRotation.y += -pointerRelative.y / (float)platformGetPlatformState()->height * camera->sensitivity;
        cameraRotation.x = fmodf(cameraRotation.x, TO_RADIANS(360));
        cameraRotation.y = clamp(cameraRotation.y, -1.5f, 1.5);
    }

    uint32_t cameraMoveSpeed = 10;
    vec3 front = vec3_normalize((vec3){{-sinf(cameraRotation.x)*cosf(cameraRotation.y), sinf(cameraRotation.y), -cosf(cameraRotation.x)*cosf(cameraRotation.y)}});
    vec3 forwardMoveAmount = vec3_scale(front, deltaTime * cameraMoveSpeed);
    vec3 rightMoveAmount = vec3_scale(vec3_normalize(vec3_cross(front, UP_DIRECTION_VEC3)), deltaTime * cameraMoveSpeed);
    vec3 upMoveAmount = vec3_scale(vec3_normalize(vec3_cross(rightMoveAmount, forwardMoveAmount)), deltaTime * cameraMoveSpeed);
    if (platformInputIsKeyDown(KEY_w)) cameraPosition = vec3_add(cameraPosition, forwardMoveAmount);
    if (platformInputIsKeyDown(KEY_s)) cameraPosition = vec3_add(cameraPosition, vec3_neg(forwardMoveAmount));
    if (platformInputIsKeyDown(KEY_a)) cameraPosition = vec3_add(cameraPosition, vec3_neg(rightMoveAmount));
    if (platformInputIsKeyDown(KEY_d)) cameraPosition = vec3_add(cameraPosition, rightMoveAmount);
    if (platformInputIsKeyDown(KEY_q)) cameraPosition = vec3_add(cameraPosition, upMoveAmount);
    if (platformInputIsKeyDown(KEY_e)) cameraPosition = vec3_add(cameraPosition, vec3_neg(upMoveAmount));

    atomicVec3SetVec3(&camera->position, cameraPosition);
    atomicVec3SetVec3(&camera->rotation, cameraRotation);
}

int main() {
    uint32_t width = 600, height = 600;
    engineInitialize("yanGameEngine - Physics Engine Test", 0, 0, width, height);
    platformWindowSetResizable(false);

    int cpuFps = 60 * 3;
    int gpuFps = 144;

    if (getenv("cpu_fps")) cpuFps = strtol(getenv("cpu_fps"), NULL, 10);
    if (getenv("gpu_fps")) gpuFps = strtol(getenv("gpu_fps"), NULL, 10);

    rendererSetFPS(gpuFps);

    double mainDt = 1.0 / cpuFps;
    double physicsDt = 1 / 60.0;
    int physicsMaxSteps = 5;

    Model* model = assetLoadGLTF("./assets/world/models/BoxAnimated", "BoxAnimated.gltf");
    Model* sphare = assetGenerateUVSphere(8, 8, 1, v4(1, 1, 1, 1));

    Scene* scene = sceneCreate();

    Entity* modelEntity = sceneCreateEntity(scene, model);
    entityCreatePhysicsBody(modelEntity);
    physicsBodyStaticSet(modelEntity->physicsBody, true);

    Light* pointLight = sceneCreatePointLight(scene, sphare, (PointLight){
        //.ambient = {{.1, .1, .1}},
        .ambient = {{1, 1, 1}},
        .position = {{5, 0, 0}},
        .diffuse = {{1, 1, 1}},
        .linear = 0.007,
        .quadratic = 0.00098
    });
    Entity* pointLightEntity = scenePointLightGetEntity(scene, pointLight);
    pointLightEntity->transform.scale = v3_all(0.1);

    sceneCameraSetPosition(scene, (vec3){{0, 0, 7}});
    rendererSetScene(scene);

    sceneEntityApplyTransform(scene);

    bool locked = false, paused = false;
    PassiveDelay lKey = passiveDelaySet(0.3);
    PassiveDelay xKey = passiveDelaySet(0.3);
    PassiveDelay escKey = passiveDelaySet(0.3);
    PassiveDelay gKey = passiveDelaySet(0.3);

    UIText* frameRateText = rendererUICreateUIText((vec3){{1, -1}}, (vec4){{1}}, 1);
    PassiveDelay frameRateTextUpdateDelay = passiveDelaySet(0.5);

    double runPhysicsAt = 0;
    TimeManager timeManager = timeManagerStart();
    while (!platformGetPlatformState()->isWindowClosed) {
        timeManagerUpdate(&timeManager);
        handleCamera(&scene->camera, timeManager.deltaTime);

        if (platformInputIsKeyDown(KEY_l) && passiveDelayIsDoneIfSoReset(&lKey)) {
            if (!locked) {
                platformPointerLock();
                locked = true;
            } else {
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

        if (platformInputIsKeyDown(KEY_g) && passiveDelayIsDoneIfSoReset(&gKey)) {
            pointLightEntity->transform.translation = v3_all(rand() % 5);
            paused ^= 1;
            runPhysicsAt = timeManager.elapsedTime;
            DEBUG(paused ? "Paused" : "Resumed");
        }

        if (!paused) {
            if (!runPhysicsAt) runPhysicsAt = timeManager.elapsedTime;
            int steps = 0;
            while (runPhysicsAt <= timeManager.elapsedTime) {
                physicsEngineRun(scene->physicsEngine, physicsDt);
                runPhysicsAt += physicsDt;
                if (++steps >= physicsMaxSteps) {
                    runPhysicsAt = timeManager.elapsedTime;
                    break;
                }
            }
        }
        sceneEntityApplyTransform(scene);
        rendererUIFixScale();
        platformPullEvent();

        if (passiveDelayIsDoneIfSoReset(&frameRateTextUpdateDelay)) {
            rendererUIPrint(frameRateText, "%.2f", 1 / timeManager.deltaTime);
            frameRateText->position.x = 1 - frameRateText->textLength * frameRateText->characters[0].size.x;
        }

        double frameTime = platformGetTime() - timeManager.lastTime;
        if (frameTime < mainDt) {
            double sleepTime = mainDt - frameTime;
            platformSleep(sleepTime);
        }
    }
    engineShutdown();
}