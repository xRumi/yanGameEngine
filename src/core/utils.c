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
void passiveDelayReset(PassiveDelay* passiveDelay) {
    passiveDelay->startTime = platformGetTime();
}
