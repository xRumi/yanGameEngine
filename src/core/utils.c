#include "utils.h"

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