#pragma once

#include "defines.h"
#include "math_types.h"
#include "hashMap.h"

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
// joints -> (0,0,0,0)
// weights -> (1,0,0,0)

typedef enum PipelineType {
    PIPELINE_TYPE_DEFAULT,
    PIPELINE_TYPE_WIREFRAME,
    PIPELINE_TYPE_MAX
} PipelineType;

typedef struct Image {
    void* data;
    uint32_t width, height;
    void* imageRendererStateRef;
} Image;

typedef struct Texture {
    Image* image;
} Texture;

typedef struct Mesh {
    Vertex* vertices;
    uint32_t* indices;
    void* meshRendererStateRef;
} Mesh;

typedef struct Material {
    Texture baseColor;
    Texture normal;
    Texture metallicRoughness;
    Texture occlusion;
    Texture emissive;
    float metallicFactor;
    float roughnessFactor;

    PipelineType pipelineType;
    Mesh* meshes;
    void* materialRendererStateRef;
} Material;

typedef struct Model {
    bool rendererLoaded;
    HashMap* images;
    HashMap* materials;
} Model;

typedef struct Entity {
    uint64_t id;
    Model* modelRef;
    mat4 transform;
} Entity;