#pragma once

#include "defines.h"

#define STR_GEN(STR) #STR,
#define ENUM_GEN(ENUM) ENUM,
#define MEMORY_TAG_GEN(GEN_TYPE) \
    GEN_TYPE(MEMORY_TAG_UNKNOWN) \
    GEN_TYPE(MEMORY_TAG_DARRAY) \
    GEN_TYPE(MEMORY_TAG_HASHSET) \
    GEN_TYPE(MEMORY_TAG_APPLICATION) \
    GEN_TYPE(MEMORY_TAG_TEXTURE) \
    GEN_TYPE(MEMORY_TAG_RENDERER) \
    GEN_TYPE(MEMORY_TAG_ENGINE) \
    GEN_TYPE(MEMORY_TAG_MEMORY_SUBSYSTEM) \
    GEN_TYPE(MEMORY_TAG_PLATFORM_SUBSYSTEM) \
    GEN_TYPE(MEMORY_TAG_MAX)

typedef enum MemoryTag {
    MEMORY_TAG_GEN(ENUM_GEN)
} MemoryTag;

void memoryInitialize();
void memoryShutdown();

void* memalloc(uint64_t size, MemoryTag tag);
void memfree(void* mem, uint64_t size, MemoryTag tag);

const char* get_memory_usage_str();