#pragma once

#define TO_RADIANS(degree) degree * M_PI / 180.0

typedef union vec2 {
    float ele[2];
    struct {
        float x, y;
    };
} vec2;

typedef union vec3 {
    float ele[3];
    struct {
        float x, y, z;
    };
} vec3;

typedef union vec4 {
    float ele[4];
    struct {
        float x, y, z, w;
    };
} vec4;

typedef union mat4 {
    float ele[4 * 4];
    float e[4][4];
    struct {
        vec4 x, y, z, w;
    };
} mat4;