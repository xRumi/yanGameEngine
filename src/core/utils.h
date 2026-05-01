#pragma once

#include "defines.h"
#include "darray.h"
#include "math_types.h"
#include <stdatomic.h>

#define DEBUG_vec3(v) DEBUG("(%.2f, %.2f, %.2f)", v.x, v.y, v.z)

char* readFile(const char* filename);
float clamp(float val, float min, float max);

typedef struct PassiveDelay {
    double startTime;
    double delay;
} PassiveDelay;
PassiveDelay passiveDelaySet(double delay);
bool passiveDelayIsDone(PassiveDelay passiveDelay);
void passiveDelayReset(PassiveDelay* passiveDelay);
bool passiveDelayIsDoneIfSoReset(PassiveDelay* passiveDelay);

typedef struct TimeManager {
    double startTime;
    double lastTime;
    double elapsedTime;
    double deltaTime;
} TimeManager;
TimeManager timeManagerStart();
void timeManagerUpdate(TimeManager* timeManager);

typedef struct AtomicMatrix {
    mat4 buffers[2]; // 0 = read, 1 = write
    atomic_flag locked;
    bool shouldYield;
} AtomicMatrix;
mat4 atomicMatrixGetMatrix(AtomicMatrix* atomicMatrix);
void atomicMatrixSetMatrix(AtomicMatrix* atomicMatrix, mat4 matrix);

void stringBuilderConcat(char** darray, const char* message, ...);
