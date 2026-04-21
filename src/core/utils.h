#pragma once

#include "defines.h"
#include "darray.h"

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

void stringBuilderConcat(char** darray, const char* message, ...);