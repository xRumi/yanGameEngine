#pragma once

#include "defines.h"

typedef struct DarrayState {
    uint64_t capacity;
    uint64_t length;
    uint64_t stride;
    enum MemoryTag memoryTag;
    uint64_t whatever;
} DarrayState;

#define _DARRAY_INITIAL_CAPACITY 2

void* _darray_create(uint64_t capacity, uint64_t stride, enum MemoryTag memoryTag);
void* _darray_create_reserve(uint64_t length, uint64_t stride, enum MemoryTag memoryTag);
void _darray_destroy(void* darray);

void* _darray_insert_at(void* darray, const void* data, uint64_t index);
void* _darray_erase_at(void* darray, uint64_t index);

DarrayState* _darray_get_state(const void* darray);

#define darray_create_memoryTag(type, memoryTag) _darray_create(_DARRAY_INITIAL_CAPACITY, sizeof(type), memoryTag)
#define darray_create(type) darray_create_memoryTag(type, MEMORY_TAG_DARRAY)
#define darray_create_reserve_memoryTag(type, length, memoryTag) _darray_create_reserve(length, sizeof(type), memoryTag)
#define darray_create_reserve(type, length) darray_create_reserve_memoryTag(type, length, MEMORY_TAG_DARRAY);
#define darray_insert_at(darray, data, index)                                                   \
    {                                                                                           \
        typeof(data) __temp = data;                                                             \
        darray = _darray_insert_at(darray, &__temp, index);                                     \
    }
#define darray_erase_at(darray, index) darray = _darray_erase_at(darray, index);

#define darray_push(darray, data) darray_insert_at(darray, data, darray_get_length(darray))
#define darray_pop(darray) darray_erase_at(darray, darray_get_length(darray) - 1)                                                                                    
#define darray_at(darray, index) ((void*)(VOID_P_TO_UCHAR_P(darray) + index*darray_get_stride(darray)))
#define darray_at_type(darray, index, type) (*(type*)darray_at(darray, index))

#define darray_get_state(darray) _darray_get_state(darray)
#define darray_get_length(darray) (darray ? _darray_get_state(darray)->length : 0)
#define darray_get_capacity(darray) _darray_get_state(darray)->capacity
#define darray_get_stride(darray) _darray_get_state(darray)->stride

#define darray_destroy(darray) _darray_destroy(darray)

extern volatile int volatile_true;
#define darray_foreach(darray, x) \
    for (int __size = darray_get_length(darray), __i = 0; __i < __size && ((x = darray_at_type(darray, __i, typeof(x))) || volatile_true); __i++)

