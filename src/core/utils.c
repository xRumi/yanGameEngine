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

mat4 atomicMatrixGetMatrix(AtomicMatrix* atomicMatrix) {
    if (!atomicMatrix->shouldYield && atomic_flag_test_and_set(&atomicMatrix->locked) == 0) {
        mat4 model = atomicMatrix->buffers[1];
        atomic_flag_clear(&atomicMatrix->locked);
        atomicMatrix->buffers[0] = model;
    }
    return atomicMatrix->buffers[0];
}

void atomicMatrixSetMatrix(AtomicMatrix* atomicMatrix, mat4 matrix) {
    atomicMatrix->shouldYield = true;
    while (atomic_flag_test_and_set(&atomicMatrix->locked) != 0);
    atomicMatrix->buffers[1] = matrix;
    atomic_flag_clear(&atomicMatrix->locked);
    atomicMatrix->shouldYield = false;
}