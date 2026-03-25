#include "asset_manager.h"
#include "asset_types.h"
#include "hashMap.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

uint32_t* load_indices(const cgltf_accessor* accessor) {
    if (!accessor || accessor->type != cgltf_type_scalar) return NULL;
    unsigned long count = accessor->count,
        offset = accessor->offset,
        stride = accessor->stride;
    uint8_t* data = (uint8_t*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset;
    uint32_t* indices = darray_create_reserve_memoryTag(uint32_t, count, MEMORY_TAG_MODEL_LOADER);
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
    Vertex* vertices = darray_create_reserve_memoryTag(Vertex, vertexCount, MEMORY_TAG_MODEL_LOADER);
    for (int i = 0; i < vertexCount; i++) {
        vertices[i] = (Vertex){
            .position = {{0, 0, 0}},
            .color = {{1, 1, 1, 1}},
            .texCoord = {{0, 0}},
            .normal = {{0, 0, 1}},
            .tangent = {{1, 0, 0, 1}},
        };
    }
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
                    vertices[j].position = position;
                    break;
                }
                case cgltf_attribute_type_color: {
                    vec4 color = { {0, 0, 0, 1} };
                    memcpy(&color, data + offset + stride * j, cgltf_calc_size(accessor->type, accessor->component_type));
                    vertices[j].color = color;
                    break;
                }
                case cgltf_attribute_type_texcoord: {
                    vec2 texCoord = { {0, 0} };
                    memcpy(&texCoord, data + offset + stride * j, cgltf_calc_size(accessor->type, accessor->component_type));
                    vertices[j].texCoord = texCoord;
                    break;
                }
                case cgltf_attribute_type_normal: {
                    vec3 normal = {};
                    memcpy(&normal, data + offset + stride * j, sizeof(vec3));
                    vertices[j].normal = normal;
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

HashMap* load_images(const char* gltf_dir, cgltf_image* images, uint32_t imageCount) {
    (void)stbi__mul2shorts_valid;
    (void)stbi__addints_valid;
    HashMap* imageHashMap = hashmap_create(100);
    char imagePath[256];
    for (uint32_t i = 0; i < imageCount; i++) {
        if (!images[i].uri) {
            WARN("Found image with null uri, skipping");
            continue;
        }
        snprintf(imagePath, 256, "%s/%s", gltf_dir, images[i].uri);
        int width = 0, height = 0, channel = 0;
        stbi_uc* pixels = stbi_load(imagePath, &width, &height, &channel, STBI_rgb_alpha);
        if (!pixels) {
            WARN("Failed to load image, skipping");
            continue;
        }
        Image* img = memalloc(sizeof(Image), MEMORY_TAG_MODEL_LOADER);
        img->height = height;
        img->width = width;
        img->data = pixels;
        hashmap_put(imageHashMap, hash_string(images[i].uri), (uint64_t)img);
    }
    return imageHashMap;
}

HashMap* load_materials(cgltf_material* gltf_material, uint32_t materialCount, HashMap* images) {
    HashMap* materials = hashmap_create(materialCount * 10);
    for (int i = 0; i < materialCount; i++) {
        Material* material = memalloc(sizeof(Material), MEMORY_TAG_MODEL_LOADER);
        if (gltf_material[i].pbr_metallic_roughness.base_color_texture.texture) {
            material->baseColor.image = (Image*)hashmap_get(images, hash_string(gltf_material[i].pbr_metallic_roughness.base_color_texture.texture->image->uri));
        }
        material->meshes = darray_create_memoryTag(Mesh, MEMORY_TAG_MODEL_LOADER);
        material->pipelineType = PIPELINE_TYPE_MESH;
        hashmap_put(materials, (uint64_t)&gltf_material[i], (uint64_t)material);
    }
    return materials;
}

Model* modelCreate(const char* gltf_dir, const char* gltf_file) {
    cgltf_options options = {};
    cgltf_data* gltf_data;
    char gltf_path[256];
    snprintf(gltf_path, 256, "%s/%s", gltf_dir, gltf_file);
    if (cgltf_parse_file(&options, gltf_path, &gltf_data) != cgltf_result_success) {
        WARN("Failed to load model \"%s\"", gltf_path);
        return NULL;
    }
    if (cgltf_load_buffers(&options, gltf_data, gltf_path) != cgltf_result_success) {
        WARN("Failed to load model buffers \"%s\"", gltf_path);
        return NULL;
    }

    Model* model = memalloc(sizeof(Model), MEMORY_TAG_MODEL_LOADER);
    model->images = load_images(gltf_dir, gltf_data->images, gltf_data->images_count);
    model->materials = load_materials(gltf_data->materials, gltf_data->materials_count, model->images);

    for (int i = 0; i < gltf_data->meshes_count; i++) {
        int primitives_count = gltf_data->meshes[i].primitives_count;
        for (int j = 0; j < primitives_count; j++) {
            cgltf_primitive* primitive = &gltf_data->meshes[i].primitives[j];

            switch (primitive->type) {
                case cgltf_primitive_type_triangles: {
                    if (!primitive->material) {
                        WARN("Material not provided for a mesh, skipping");
                        break;
                    }
                    if (!hashmap_has(model->materials, (uint64_t)primitive->material)) {
                        WARN("Material not found for a mesh, skipping");
                        break;
                    }

                    Mesh mesh = {};

                    mesh.indices = load_indices(primitive->indices);
                    if (mesh.indices == NULL) FATAL("[%s] Failed to load indices", gltf_path);

                    mesh.vertices = load_vertices(primitive->attributes, primitive->attributes_count);
                    if (mesh.vertices == NULL) FATAL("[%s] Failed to load vertices", gltf_path);

                    Material* material = (Material*)hashmap_get(model->materials, (uint64_t)primitive->material);
                    darray_push(material->meshes, mesh);

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