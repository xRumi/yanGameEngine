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
    if (hashMap == NULL) return;
    if (hashmap_has(hashMap, key)) {
        hashmap_swap(hashMap, key, val);
        return;
    }
    hashMap->size++;
    uint64_t index = hashFunction(key, hashMap->capacity);
    if (hashMap->nodes[index] == NULL) hashMap->nodes[index] = darray_create_memoryTag(HashNode, MEMORY_TAG_HASHMAP);
    HashNode node = {key, val};
    darray_push(hashMap->nodes[index], node);
}
uint64_t hashmap_get(HashMap* hashMap, uint64_t key) {
    if (hashMap == NULL) return 0;
    uint64_t index = hashFunction(key, hashMap->capacity);
    if (hashMap->nodes[index] == NULL) return 0;
    uint64_t nodeCount = darray_get_length(hashMap->nodes[index]);
    for (int i = 0; i < nodeCount; i++)
        if (hashMap->nodes[index][i].key == key) return hashMap->nodes[index][i].val;
    return 0;
}
uint64_t hashmap_swap(HashMap* hashMap, uint64_t key, uint64_t val) {
    if (hashMap == NULL) return 0;
    uint64_t index = hashFunction(key, hashMap->capacity);
    if (hashMap->nodes[index] == NULL) return 0;
    uint64_t nodeCount = darray_get_length(hashMap->nodes[index]);
    for (int i = 0; i < nodeCount; i++)
        if (hashMap->nodes[index][i].key == key) {
            uint64_t temp = hashMap->nodes[index][i].val;
            hashMap->nodes[index][i].val = val;
            return temp;
        }
    return 0;
}
bool hashmap_has(HashMap* hashMap, uint64_t key) {
    if (hashMap == NULL) return 0;
    uint64_t index = hashFunction(key, hashMap->capacity);
    if (hashMap->nodes[index] == NULL) return false;
    uint64_t nodeCount = darray_get_length(hashMap->nodes[index]);
    for (int i = 0; i < nodeCount; i++)
        if (hashMap->nodes[index][i].key == key) return true;
    return false;
}
void hashmap_remove(HashMap* hashMap, uint64_t key) {
    if (hashMap == NULL) return;
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
    if (hashMap == NULL) return;
    uint64_t capacity = hashMap->capacity;
    for (uint64_t i = 0; i < capacity; i++) darray_destroy(hashMap->nodes[i]);
    darray_destroy(hashMap->nodes);
    memfree(hashMap, sizeof(HashNode), MEMORY_TAG_HASHMAP);
}

uint64_t hash_string(const char* str) {
    uint64_t hash = 0;
    int length = strlen(str);
    for (int i = 0; i < length; i++)
        hash += (uint64_t)str[i] << (i % 8);
    return hash;
}