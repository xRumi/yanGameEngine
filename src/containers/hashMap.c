#include "hashMap.h"

uint64_t hashFunction(uint64_t key, uint64_t capacity) {
    return key % capacity;
}

HashMap* hashmap_create(uint64_t capacity) {
    HashMap* hashMap = memalloc(sizeof(HashNode), MEMORY_TAG_HASHMAP);
    hashMap->capacity = capacity;
    hashMap->size = 0;
    hashMap->nodes = darray_create_reserve_memoryTag(HashNode*, capacity, MEMORY_TAG_HASHMAP);
    return hashMap;
}
void hashmap_put(HashMap* hashMap, uint64_t key, uint64_t val) {
    if (hashmap_has(hashMap, key)) return;
    hashMap->size++;
    uint64_t index = hashFunction(key, hashMap->capacity);
    if (hashMap->nodes[index] == NULL) hashMap->nodes[index] = darray_create_memoryTag(HashNode, MEMORY_TAG_HASHMAP);
    HashNode node = {key, val};
    darray_push(hashMap->nodes[index], node);
}
uint64_t hashmap_get(HashMap* hashMap, uint64_t key) {
    uint64_t index = hashFunction(key, hashMap->capacity);
    if (hashMap->nodes[index] == NULL) return 0;
    uint64_t nodeCount = darray_get_length(hashMap->nodes[index]);
    for (int i = 0; i < nodeCount; i++)
        if (hashMap->nodes[index][i].key == key) return hashMap->nodes[index][i].val;
    return 0;
}
bool hashmap_has(HashMap* hashMap, uint64_t key) {
    uint64_t index = hashFunction(key, hashMap->capacity);
    if (hashMap->nodes[index] == NULL) return false;
    uint64_t nodeCount = darray_get_length(hashMap->nodes[index]);
    for (int i = 0; i < nodeCount; i++)
        if (hashMap->nodes[index][i].key == key) return true;
    return false;
}
void hashmap_remove(HashMap* hashMap, uint64_t key) {
    uint64_t index = hashFunction(key, hashMap->capacity);
    if (hashMap->nodes[index] == NULL) return;
    uint64_t nodeCount = darray_get_length(hashMap->nodes[index]);
    for (int i = 0; i < nodeCount; i++)
        if (hashMap->nodes[index][i].key == key) {
            darray_erase_at(hashMap->nodes[index], i);
            hashMap->size--;
            break;
        }
}
void hashmap_destroy(HashMap* hashMap) {
    uint64_t capacity = hashMap->capacity;
    for (uint64_t i = 0; i < capacity; i++) darray_destroy(hashMap->nodes[i]);
    darray_destroy(hashMap->nodes);
    memfree(hashMap, sizeof(HashNode), MEMORY_TAG_HASHMAP);
}

uint64_t hash_string(const char* str) {
    uint64_t hash = 0;
    int length = strlen(str);
    int uints = length / 4;
    for (int i = 0; i < uints; i++)
        hash += *(uint64_t*)(str + i*4);
    for (int i = uints * 4; i < length; i++)
        hash += (uint64_t)(str[i]) << (length - i);
    return hash;
}