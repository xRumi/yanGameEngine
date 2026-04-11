#pragma once

#include "defines.h"
#include "darray.h"

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