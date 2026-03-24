#include "darray.h"

void* _darray_create(uint64_t capacity, uint64_t stride, enum MemoryTag memoryTag) {
    unsigned char* block = (unsigned char*)memalloc(sizeof(DarrayState) + (capacity * stride), memoryTag);
    DarrayState* state = (DarrayState *)block;
    state->capacity = capacity;
    state->stride = stride;
    state->memoryTag = memoryTag;
    return (block + sizeof(DarrayState));
};

void* _darray_create_reserve(uint64_t length, uint64_t stride, enum MemoryTag memoryTag) {
    void* darray = _darray_create(length, stride, memoryTag);
    _darray_get_state(darray)->length = length;
    return darray;
}

void _darray_destroy(void* darray) {
    if (darray == NULL) return;
    DarrayState* state = _darray_get_state(darray);
    memfree((void*)state, sizeof(DarrayState) + (state->capacity * state->stride), state->memoryTag);
};

void* _darray_resize_capacity(void* darray, uint64_t capacity) {
    DarrayState* state = _darray_get_state(darray);
    void* new_darray = _darray_create(capacity, state->stride, state->memoryTag);
    DarrayState* new_state = _darray_get_state(new_darray);
    new_state->length = state->length;
    new_state->whatever = state->whatever;
    memcpy(new_darray, darray, new_state->length * new_state->stride);
    _darray_destroy(darray);
    return new_darray;
}

void* _darray_resize_length(void* darray, uint64_t length) {
    DarrayState* state = _darray_get_state(darray);
    if (state->length < length) {
        if (state->capacity < length) {
            darray = _darray_resize_capacity(darray, length);
            state = _darray_get_state(darray);
        }
    }
    state->length = length;
    return darray;
}

void* _darray_insert_at(void* darray, const void* data, uint64_t index) {
    DarrayState* state = _darray_get_state(darray);
    if (state->length < index) {
        darray = _darray_resize_length(darray, index + 1);
        memcpy(VOID_P_TO_UCHAR_P(darray) + index*state->stride, data, state->stride);
        return darray;
    }
    if (state->length == state->capacity) {
        darray = _darray_resize_capacity(darray, state->capacity * 2);
        state = _darray_get_state(darray);
    }
    if (index != state->length)
        memmove(VOID_P_TO_UCHAR_P(darray) + (index + 1)*state->stride, VOID_P_TO_UCHAR_P(darray) + index*state->stride, (state->length - index)*state->stride);
    memcpy(VOID_P_TO_UCHAR_P(darray) + index*state->stride, data, state->stride);
    state->length++;
    return darray;
};
void* _darray_erase_at(void* darray, uint64_t index) {
    DarrayState* state = _darray_get_state(darray);
    if (state->length == 0 || state->length <= index) {
        FATAL("darray - erase_at index out of bounds");
        return darray;
    }
    memmove(VOID_P_TO_UCHAR_P(darray) + index*state->stride, VOID_P_TO_UCHAR_P(darray) + (index + 1)*state->stride, (state->length - index)*state->stride);
    state->length--;
    return darray;
}

DarrayState* _darray_get_state(const void* darray) {
    return (DarrayState*)(VOID_P_TO_UCHAR_P(darray) - sizeof(DarrayState));
}