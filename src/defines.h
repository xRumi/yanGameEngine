#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>

#include "logger.h"
#include "memory.h"


#define VOID_P_TO_UCHAR_P(v) (unsigned char*)v
#define CARRAY_SIZE(v) sizeof(v)/sizeof(v[0])

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? a : b)
#define ABS(a) (a > 0 ? a : -a)