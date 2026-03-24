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

#define hashmap_foreach(hashMap, type, var, action)                                 \
    {                                                                               \
        int __size = hashMap->capacity;                                             \
        for (int __i = 0; __i < __size; __i++) {                                    \
            if (hashMap->nodes[__i] == NULL) continue;                              \
            int nodeCount = darray_get_length(hashMap->nodes[__i]);                 \
            for (int __j = 0; __j < nodeCount; __j++) {                             \
                typeof(type) var = (typeof(type))hashMap->nodes[__i][__j].val;      \
                action                                                              \
            }                                                                       \
        }                                                                           \
    }

