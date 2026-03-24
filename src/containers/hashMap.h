#pragma once

#include "defines.h"
#include "darray.h"

typedef struct HashNode {
    uint64_t key;
    uint64_t val;
    struct HashNode* next;
} HashNode;

typedef struct HashMap {
    uint64_t capacity, size;
    HashNode** nodes;
} HashMap;

HashMap* hashmap_create(uint64_t capacity);
void hashmap_put(HashMap* hashMap, uint64_t key, uint64_t val);
uint64_t hashmap_get(HashMap* hashMap, uint64_t key);
bool hashmap_has(HashMap* hashMap, uint64_t key);
void hashmap_remove(HashMap* hashMap, uint64_t key);
void hashmap_destroy(HashMap* hashMap);

uint64_t hash_string(const char* str);

extern volatile int volatile_true;
#define hashmap_foreach(hashMap, x) \
    for (int __size = hashMap->capacity, __i = 0; __i < __size; __i++) \
        for (int __nodeSize = darray_get_length(hashMap->nodes[__i]), __j = 0; __j < __nodeSize && ((x = (typeof(x))hashMap->nodes[__i][__j].val) || volatile_true); __j++)


