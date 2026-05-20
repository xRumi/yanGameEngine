#include "utils.h"
#include "platform.h"

char* readFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        ERROR("Failed to open file %s", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size < 0) {
        FATAL("Failed to ftell file %s", filename);
    }
    char* data = darray_create_reserve(char, size);
    fseek(file, 0, SEEK_SET);
    fread(data, 1, size, file);
    fclose(file);
    return data;
};

float clamp(float val, float min, float max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

PassiveDelay passiveDelaySet(double delay) {
    return (PassiveDelay){
        .startTime = platformGetTime(),
        .delay = delay
    };
}
bool passiveDelayIsDone(PassiveDelay passiveDelay) {
    return (platformGetTime() - passiveDelay.startTime) >= passiveDelay.delay;
}
bool passiveDelayIsDoneIfSoReset(PassiveDelay* passiveDelay) {
    bool isDone = passiveDelayIsDone(*passiveDelay);
    if (isDone) passiveDelayReset(passiveDelay);
    return isDone;
}
void passiveDelayReset(PassiveDelay* passiveDelay) {
    passiveDelay->startTime = platformGetTime();
}

TimeManager timeManagerStart() {
    return (TimeManager) {
        .startTime = platformGetTime(),
        .lastTime = platformGetTime(),
    };
}
void timeManagerUpdate(TimeManager* timeManager) {
    double currentTime = platformGetTime();
    timeManager->elapsedTime = currentTime - timeManager->startTime;
    timeManager->deltaTime = currentTime - timeManager->lastTime;
    timeManager->lastTime = currentTime;
}

void stringBuilderConcat(char** darray, const char* message, ...) {
    const int messageLimit = 32768;
    char output[messageLimit];
    va_list args;
    va_start(args, message);
    vsnprintf(output, messageLimit, message, args);
    va_end(args);
    for (int i = 0; output[i]; i++) {
        darray_push(*darray, output[i]);
    }
}

#define implementAtomicData(name, type) \
type CONCAT(atomic##name, Get##name)(Atomic##name* atomicData) { \
    if (!atomicData->shouldYield && atomic_flag_test_and_set(&atomicData->locked) == 0) { \
        type data = atomicData->buffers[1]; \
        atomic_flag_clear(&atomicData->locked); \
        atomicData->buffers[0] = data; \
    } \
    return atomicData->buffers[0]; \
} \
void CONCAT(atomic##name, Set##name)(Atomic##name* atomicData, type value) { \
    atomicData->shouldYield = true; \
    while (atomic_flag_test_and_set(&atomicData->locked) != 0); \
    atomicData->buffers[1] = value; \
    atomic_flag_clear(&atomicData->locked); \
    atomicData->shouldYield = false; \
}

implementAtomicData(Matrix, mat4);
implementAtomicData(Vec3, vec3);