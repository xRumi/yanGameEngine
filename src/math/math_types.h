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

typedef struct Vertex {
    vec3 position;
    vec4 color;
    vec2 texCoord;
    vec3 normal;
    vec4 tangent;
} Vertex;

// typedef struct Vertex {
//     uint16_t joints[4];
//     float weights[4];
// } Vertex;

// default values
// normal  -> (0,0,1)
// tangent -> (1,0,0,1)
// texcoord -> (0,0)
// color -> (1,1,1,1)
// joints -> (0,0,0,0)
// weights -> (1,0,0,0)