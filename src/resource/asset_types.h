#pragma once

#include "defines.h"
#include "math_types.h"
#include "hashMap.h"

typedef enum PipelineType {
    PIPELINE_TYPE_MESH,
    PIPELINE_TYPE_MAX
} PipelineType;

typedef struct Image {
    void* data;
    uint32_t width, height;
} Image;

typedef struct Texture {
    bool useTexture;
    uint64_t imageHash;
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

    Mesh* meshes;
    PipelineType pipelineType;
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