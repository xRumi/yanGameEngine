#include "engine.h"
#include "asset_types.h"
#include "asset_manager.h"
#include "rendererAPI.h"
#include "utils.h"

typedef struct TwoPillar {
    Entity* upper;
    Entity* lower;
    bool scored;
} TwoPillar;

void randomizeTwoPillarTranslation(TwoPillar* pillar, float minHeight, float maxHeight) {
    int height = clamp(rand() % 10, minHeight, maxHeight);
    float center = 3 * (rand() / (double)RAND_MAX) - 1.5;
    entityTransformSetTranslationY(pillar->upper, center + height / 2.0 + 2);
    entityTransformSetTranslationY(pillar->lower, center - height / 2.0 - 2);
}

TwoPillar* createTwoPillarArray(Model* pillar, Scene* scene, int count, float start, float distance) {
    TwoPillar* array = darray_create_resized(TwoPillar, count);
    for (int i = 0; i < count; i++) {
        Entity* upperPillar = sceneCreateEntity(scene, pillar);
        entityTransformSetTranslation(upperPillar, (vec3){{start + distance * i, 2, 0.5}});
        entityCreatePhysicsBody(upperPillar);
        physicsBodyGravitySet(upperPillar->physicsBody, 0);

        Entity* lowerPillar = sceneCreateEntity(scene, pillar);
        entityTransformSetTranslation(lowerPillar, (vec3){{start + distance * i, -2, 0.5}});
        entityCreatePhysicsBody(lowerPillar);
        physicsBodyGravitySet(lowerPillar->physicsBody, 0);

        array[i] = (TwoPillar) {
            .upper = upperPillar,
            .lower = lowerPillar
        };
        randomizeTwoPillarTranslation(&array[i], 1, 2);
    }
    return array;
}

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

    Model* background = assetGenerateRectangle((vec3){{-3, 5}}, (vec3){{-3, -5}}, (vec3){{3, -5}}, (vec3){{3, 5}}, "./assets/world/models/FlappyBird/bg.png");
    Model* pillar = assetGenerateRectangle((vec3){{-0.2, 2}}, (vec3){{-0.2, -2}}, (vec3){{0.2, -2}}, (vec3){{0.2, 2}}, "./assets/world/models/FlappyBird/pillar.jpg");
    Model* bird = assetGenerateUVSphere(8, 8, 0.13, (vec4){{1, 0, 0, 1}});

    Scene* scene = sceneCreate();

    Entity* backgroundEntity = sceneCreateEntity(scene, background);
    (void)backgroundEntity;

    TwoPillar* twoPillarArray = createTwoPillarArray(pillar, scene, 4, 0, 2.5);

    Entity* birdEntity = sceneCreateEntity(scene, bird);
    entityTransformSetTranslation(birdEntity, (vec3){{-1.6, 0, 0.5}});
    entityCreatePhysicsBody(birdEntity);
    physicsBodyGravitySet(birdEntity->physicsBody, 5);

    Model* lightSphare = assetGenerateUVSphere(8, 8, 1, v4(1, 1, 1, 1));
    Light* pointLight = sceneCreatePointLight(scene, lightSphare, (PointLight){
        .ambient = {{.1, .1, .1}},
        //.ambient = {{1, 1, 1}},
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

    bool locked = false;
    PassiveDelay lKey = passiveDelaySet(0.3);
    PassiveDelay xKey = passiveDelaySet(0.3);
    PassiveDelay escKey = passiveDelaySet(0.3);
    PassiveDelay gKey = passiveDelaySet(0.2);

    UIText* scoreText = rendererUICreateUIText((vec3){{-1, -1}}, (vec4){{0, 1, 0, 1}}, 1.2);
    rendererUIPrint(scoreText, "%d", 0);
    UIText* gameOverText = rendererUICreateUIText((vec3){{-0.4, 0}}, (vec4){{1, 0, 0, 1}}, 1.4);

    UIText* frameRateText = rendererUICreateUIText((vec3){{1, -1}}, (vec4){{1}}, 1);
    PassiveDelay frameRateTextUpdateDelay = passiveDelaySet(0.5);

    bool gameOver = false, paused = true;
    int score = 0;

    WARN("Press g to start playing..");

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

        if (paused) {
            if (platformInputIsKeyDown(KEY_g) && passiveDelayIsDoneIfSoReset(&gKey)) {
                paused = false;
            }
        }

        if (!gameOver && !paused) {
            if (platformInputIsKeyDown(KEY_g) && passiveDelayIsDoneIfSoReset(&gKey)) {
                playSound("./assets/sounds/sfx_wing.wav");
                birdEntity->physicsBody->velocity.y = 3;
            }
            TwoPillar* piller;
            darray_foreach(twoPillarArray, piller) {
                if (piller->upper->transform.translation.x <= birdEntity->transform.translation.x && !piller->scored) {
                    score++;
                    piller->scored = true;
                    playSound("./assets/sounds/sfx_point.wav");
                    rendererUIPrint(scoreText, "%d", score);
                }
                if (piller->upper->transform.translation.x <= -5) {
                    piller->upper->transform.translation.x = 5;
                    piller->lower->transform.translation.x = 5;
                    randomizeTwoPillarTranslation(piller, 1.3, 2.1);
                    piller->scored = false;
                }
                piller->upper->physicsBody->velocity.x = clamp(piller->upper->physicsBody->velocity.x - 0.015 * timeManager.deltaTime, -3, -1.5);
                piller->lower->physicsBody->velocity.x = piller->upper->physicsBody->velocity.x;
                if (isCollisionSphereToAabb(birdEntity->physicsBody->collider, piller->upper->physicsBody->collider) || isCollisionSphereToAabb(birdEntity->physicsBody->collider, piller->lower->physicsBody->collider)) {
                    gameOver = true;
                }
            }
            if (birdEntity->transform.translation.y > 2.75 || birdEntity->transform.translation.y < -2.75 || gameOver) {
                gameOver = true;
                playSound("./assets/sounds/sfx_hit.wav");
                WARN("GAME OVER\n\tPress ESC to close..");
            }
            if (gameOver) {
                rendererUIPrint(gameOverText, "Game Over");
                continue;
            }

            if (!runPhysicsAt) runPhysicsAt = timeManager.elapsedTime;
            int steps = 0;
            while (runPhysicsAt <= timeManager.elapsedTime) {
                physicsEngineRun(scene->physicsEngine, physicsDt);
                runPhysicsAt += physicsDt;
                if (++steps >= 5) {
                    runPhysicsAt = timeManager.elapsedTime;
                    break;
                }
            }
            sceneEntityApplyTransform(scene);
        }
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
