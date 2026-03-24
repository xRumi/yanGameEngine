#include "engine.h"
#include "emath.h"
#include "asset_types.h"
#include "asset_manager.h"
#include "rendererAPI.h"
#include <unistd.h>

#include "hashMap.h"

int main() {
    engineInitialize("yanGameEngine - Triangle");
    rendererSetFPS(144);

    Model* boxModel = modelCreate("./assets/world/models/BoxTextured", "BoxTextured.gltf");
    Entity* box1 = entityCreate(boxModel);
    Entity* box2 = entityCreate(boxModel);

    rendererAddEntity(box1);
    rendererAddEntity(box2);

    double startTime = platformGetTime();
    while (!platformGetPlatformState()->platformWindowClosed) {
        double elapsedTime = platformGetTime() - startTime;
        (void) elapsedTime;

        mat4 rotate = mat4_mul(mat4_rotation_x(35 * elapsedTime), mat4_rotation_y(35 * elapsedTime));

        mat4 move = mat4_translation(-1, 0, 0);
        entityResetTransform(box1);
        entityApplyTransform(box1, move);
        entityApplyTransform(box1, rotate);

        rotate = mat4_mul(mat4_rotation_x(85 * elapsedTime), mat4_rotation_y(85 * elapsedTime));
        move = mat4_translation(1, 0, 0);
        entityResetTransform(box2);
        entityApplyTransform(box2, move);
        entityApplyTransform(box2, rotate);

        platformPullEvent();
        platformSleep(1.0 / 60.0);
    }

    engineShutdown();
}
