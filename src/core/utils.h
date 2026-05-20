#pragma once

#include "defines.h"
#include "darray.h"
#include "math_types.h"
#include <stdatomic.h>

#define DEBUG_vec3(v) DEBUG("(%.2f, %.2f, %.2f)", v.x, v.y, v.z)

char* readFile(const char* filename);
float clamp(float val, float min, float max);
void stringBuilderConcat(char** darray, const char* message, ...);

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

#define CONCAT(a, b) a##b
#define defineAtomicData(name, type)  \
typedef struct Atomic##name {   \
    type buffers[2];            \
    atomic_flag locked;         \
    bool shouldYield;           \
} Atomic##name;                 \
type CONCAT(atomic##name, Get##name)(Atomic##name* atomic##name); \
void CONCAT(atomic##name, Set##name)(Atomic##name* atomic##name, type value);
// buffers[0] = read, buffers[1] = write
defineAtomicData(Matrix, mat4);
defineAtomicData(Vec3, vec3);