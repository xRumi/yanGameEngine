#pragma once

#include "defines.h"
#include "math_types.h"
#include "darray.h"

typedef struct Mesh {
    Vertex* vertices;
    uint32_t* indices;
} Mesh;

typedef struct Model {
    Mesh* mesh;
} Model;

Model* load_model(const char* gltf_path);