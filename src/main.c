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
    TwoPillar* array = darray_create_reserve(TwoPillar, count);
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

        entitySetHidden(upperPillar, false);
        entitySetHidden(lowerPillar, false);
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
    double targetFrameTime = 1 / 120.0;
    engineInitialize("yanGameEngine - Physics Engine Test", 0, 0, width, height);
    rendererSetFPS(144);

    Model* background = assetGenerateRectangle((vec3){{-3, 5}}, (vec3){{-3, -5}}, (vec3){{3, -5}}, (vec3){{3, 5}}, "./assets/world/models/FlappyBird/bg.png");
    Model* pillar = assetGenerateRectangle((vec3){{-0.2, 2}}, (vec3){{-0.2, -2}}, (vec3){{0.2, -2}}, (vec3){{0.2, 2}}, "./assets/world/models/FlappyBird/pillar.jpg");
    Model* bird = assetGenerateUVSphere(8, 8, 0.13, (vec4){{1, 0, 0, 1}});

    Scene* scene = sceneCreate();

    Entity* backgroundEntity = sceneCreateEntity(scene, background);
    entitySetHidden(backgroundEntity, false);

    TwoPillar* twoPillarArray = createTwoPillarArray(pillar, scene, 4, 0, 2.5);

    Entity* birdEntity = sceneCreateEntity(scene, bird);
    entityTransformSetTranslation(birdEntity, (vec3){{-1.6, 0, 0.5}});
    entityCreatePhysicsBody(birdEntity);
    physicsBodyGravitySet(birdEntity->physicsBody, 5);
    entitySetHidden(birdEntity, false);

    sceneAddDirectionalLight(scene)->ambient = (vec4){{1, 1, 1, 1}};
    sceneCameraSetPosition(scene, (vec3){{0, 0, 7}});
    rendererSetScene(scene);

    sceneEntityApplyTransform(scene);

    bool locked = false;
    PassiveDelay lKey = passiveDelaySet(0.3);
    PassiveDelay xKey = passiveDelaySet(0.3);
    PassiveDelay escKey = passiveDelaySet(0.3);
    PassiveDelay gKey = passiveDelaySet(0.2);

    bool gameOver = false, paused = true;
    int score = 0;

    WARN("Press g to start playing..");

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
                birdEntity->physicsBody->velocity.y = 3;
            }
            TwoPillar* piller;
            darray_foreach(twoPillarArray, piller) {
                if (piller->upper->transform.translation.x <= birdEntity->transform.translation.x && !piller->scored) {
                    score++;
                    piller->scored = true;
                    DEBUG("score = %d", score);
                }
                if (piller->upper->transform.translation.x <= -5) {
                    piller->upper->transform.translation.x = 5;
                    piller->lower->transform.translation.x = 5;
                    randomizeTwoPillarTranslation(piller, 1.3, 2.1);
                    piller->scored = false;
                }
                entityPhysicsBodyAddVelocity(piller->upper, (vec3){{-0.015 * targetFrameTime}});
                entityPhysicsBodyAddVelocity(piller->lower, (vec3){{-0.015 * targetFrameTime}});
                piller->upper->physicsBody->velocity.x = clamp(piller->upper->physicsBody->velocity.x, -3, -1.5);
                piller->lower->physicsBody->velocity.x = clamp(piller->lower->physicsBody->velocity.x, -3, -1.5);
                if (isCollisionSphereToAabb(birdEntity->physicsBody->collider, piller->upper->physicsBody->collider) || isCollisionSphereToAabb(birdEntity->physicsBody->collider, piller->lower->physicsBody->collider)) {
                    gameOver = true;
                }
            }
            if (birdEntity->transform.translation.y > 2.75 || birdEntity->transform.translation.y < -2.75 || gameOver) {
                gameOver = true;
                WARN("GAME OVER\n\tPress ESC to close..");
            }
            if (gameOver) continue;

            physicsEngineRun(scene->physicsEngine, 1.0 / 120.0);
            sceneEntityApplyTransform(scene);
        }
        platformPullEvent();

        double frameTime = platformGetTime() - timeManager.lastTime;
        if (frameTime < targetFrameTime) {
            double sleepTime = targetFrameTime - frameTime;
            platformSleep(sleepTime);
        }
    }
    engineShutdown();
}
