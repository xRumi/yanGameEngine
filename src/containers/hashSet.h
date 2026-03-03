#pragma once

#include "defines.h"
#include "darray.h"

void* _hashSet_create(uint64_t length, uint64_t stride);
void _hashSet_destroy(void* hashSet);


