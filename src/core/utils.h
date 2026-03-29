#pragma once

#include "defines.h"
#include "darray.h"

typedef struct PassiveDelay {
    double startTime;
    double delay;
} PassiveDelay;

char* readFile(const char* filename);
float clamp(float val, float min, float max);

PassiveDelay passiveDelaySet(double delay);
bool passiveDelayIsDone(PassiveDelay passiveDelay);
void passiveDelayReset(PassiveDelay* passiveDelay);
