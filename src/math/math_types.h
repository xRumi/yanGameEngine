#pragma once

typedef struct vec2 {
    float ele[2];
} vec2;

typedef struct vec3 {
    float ele[3];
} vec3;

typedef struct vec4 {
    float ele[4];
} vec4;

typedef struct mat4 {
    float ele[4 * 4];
} mat4;

typedef struct Vertex {
    vec3 position;
    vec4 color;
    vec2 texCoord;
} Vertex;

// typedef struct Vertex {
//     float normal[3];
//     float tangent[4];
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