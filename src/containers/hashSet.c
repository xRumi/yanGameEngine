#include "hashSet.h"

int hashFunction(int key, int capacity) {
    if (key < 0) key = -key;
    return key % capacity;
}

void* _hashSet_create(uint64_t length, uint64_t stride) {
    void* hashSet = darray_create_reserve(void*, length);
    DarrayState* state = darray_get_state(hashSet);
    state->whatever = stride;
    // TODO: hashSet
    return NULL;
}
void _hashSet_destroy(void* hashSet);