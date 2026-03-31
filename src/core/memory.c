#include "memory.h"

const char* memory_tag_strings[] = {
    MEMORY_TAG_GEN(STR_GEN)
};

struct {
    uint64_t taggedAllocation[MEMORY_TAG_MAX];
} internalStateMemory;

#define memory_usage_str_length 1024
char memory_usage_str[memory_usage_str_length];

void* memalloc(uint64_t size, MemoryTag tag) {
    internalStateMemory.taggedAllocation[MEMORY_TOTAL_ALLOCATED] += size;
    internalStateMemory.taggedAllocation[tag] += size;
    void* mem = malloc(size); // TODO: add alignment or maybe some new system
    memset(mem, 0, size);
    return mem;
};
void memfree(void* mem, uint64_t size, MemoryTag tag) {
    internalStateMemory.taggedAllocation[MEMORY_TOTAL_ALLOCATED] -= size;
    internalStateMemory.taggedAllocation[tag] -= size;
    free(mem);
};

// reused string
const char* get_memory_usage_str() {
    int offset = 0;
    for (int i = 0; i < MEMORY_TAG_MAX; i++) {
        uint64_t size = internalStateMemory.taggedAllocation[i];
        if (size == 0) continue;
        int GiB = size / (1024 * 1024 * 1024);
        size %= 1024 * 1024 * 1024;
        int MiB = size / (1024 * 1024);
        size %= 1024 * 1024;
        int KiB = size / 1024;
        size %= 1024;
        offset += snprintf(memory_usage_str + offset, memory_usage_str_length - offset, "\t%-35s", memory_tag_strings[i]);
        if (GiB) offset += snprintf(memory_usage_str + offset, memory_usage_str_length - offset, " %d GiB,", GiB);
        if (MiB) offset += snprintf(memory_usage_str + offset, memory_usage_str_length - offset, " %d MiB,", MiB);
        if (KiB) offset += snprintf(memory_usage_str + offset, memory_usage_str_length - offset, " %d KiB,", KiB);
        offset += snprintf(memory_usage_str + offset, memory_usage_str_length - offset, " %ld Byte\n", size);
    }
    memory_usage_str[offset] = 0;
    return memory_usage_str;
};