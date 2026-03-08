#include "model_loader.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"

uint32_t* load_indices(const cgltf_accessor* accessor) {
    if (!accessor || accessor->type != cgltf_type_scalar) return NULL;
    unsigned long count = accessor->count,
        offset = accessor->offset,
        stride = accessor->stride;
    uint8_t* data = (uint8_t*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset;
    uint32_t* indices = darray_create_reserve(uint32_t, count);
    for (int i = 0; i < count; i++) {
        uint8_t* nthData = data + offset + stride * i;
        uint32_t index = 0;
        if (accessor->component_type == cgltf_component_type_r_32u) index = *(uint32_t*)(nthData);
        else if (accessor->component_type == cgltf_component_type_r_16u) index = *(uint16_t*)(nthData);
        else {
            WARN("Invalid indices component type, skipping");
            continue;
        }
        indices[i] = index;
    }
    return indices;
}

Vertex* load_vertices(const cgltf_attribute* attributes, uint32_t attributeCount) {
    if (!attributes) return NULL;
    uint32_t vertexCount = attributes[0].data->count;
    Vertex* vertices = darray_create_reserve(Vertex, vertexCount);
    for (int i = 0; i < attributeCount; i++) {
        cgltf_attribute_type attributeType = attributes[i].type;
        cgltf_accessor* accessor = attributes[i].data;

        uint8_t* data = (uint8_t*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset;

        unsigned long count = accessor->count,
            offset = accessor->offset,
            stride = accessor->stride;

        bool skipAttribute = false;

        for (int j = 0; j < count; j++) {
            switch (attributeType) {
                case cgltf_attribute_type_position: {
                    vec3 position = {};
                    memcpy(&position, data + offset + stride * j, sizeof(vec3));
                    position.ele[0] /= 2;
                    position.ele[1] /= 2;
                    position.ele[2] /= 2;
                    vertices[j].position = position;
                    break;
                }
                case cgltf_attribute_type_color: {
                    vec4 color = { {0, 0, 0, 1} };
                    memcpy(&color, data + offset + stride * j, cgltf_calc_size(accessor->type, accessor->component_type));
                    vertices[j].color = color;
                    break;
                }
                default: {
                    WARN("Unknown attribute type \"%s\" provided", attributes[i].name);
                    skipAttribute = true;
                }
            }
            if (skipAttribute) break;
        }
    }
    return vertices;
}

Model* load_model(const char* gltf_path) {
    cgltf_options options = {};
    cgltf_data* gltf_data;
    if (cgltf_parse_file(&options, gltf_path, &gltf_data) != cgltf_result_success) {
        WARN("Failed to load model \"%s\"", gltf_path);
        return NULL;
    }
    if (cgltf_load_buffers(&options, gltf_data, gltf_path) != cgltf_result_success) {
        WARN("Failed to load model buffers \"%s\"", gltf_path);
        return NULL;
    }

    Model* model = memalloc(sizeof(Model), MEMORY_TAG_MODEL_LOADER);
    model->mesh = darray_create_reserve(Mesh, gltf_data->meshes_count);

    for (int i = 0; i < gltf_data->meshes_count; i++) {
        Mesh* mesh = &model->mesh[i];

        int primitives_count = gltf_data->meshes[i].primitives_count;
        for (int j = 0; j < primitives_count; j++) {
            cgltf_primitive* primitive = &gltf_data->meshes[i].primitives[j];

            switch (primitive->type) {
                case cgltf_primitive_type_triangles: {
                    mesh->indices = load_indices(primitive->indices);
                    if (mesh->indices == NULL) FATAL("[%s] Failed to load indices", gltf_path);

                    mesh->vertices = load_vertices(primitive->attributes, primitive->attributes_count);
                    if (mesh->vertices == NULL) FATAL("[%s] Failed to load vertices", gltf_path);

                    break;
                }
                default: {
                    WARN("[%s] Unknown primitive provided", gltf_path);
                }
            }

        }
    }
    cgltf_free(gltf_data);
    return model;
}