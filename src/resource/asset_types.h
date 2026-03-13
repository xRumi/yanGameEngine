#pragma once

#include "defines.h"
#include "math_types.h"

typedef enum PipelineType {
    PIPELINE_TYPE_MESH,
    PIPELINE_TYPE_MAX
} PipelineType;

typedef struct Mesh {
    Vertex* vertices;
    uint32_t* indices;
    void* rendererStateRef;
    PipelineType pipelineType;
} Mesh;

typedef struct Model {
    bool rendererLoaded;
    Mesh* mesh;
} Model;

typedef struct Entity {
    Model* modelRef;
    mat4 transform;
} Entity;