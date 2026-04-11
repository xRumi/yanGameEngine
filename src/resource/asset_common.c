#include "asset_manager.h"

void load_default_images(HashMap* images) {
    Image* image0 = memalloc(sizeof(Image), MEMORY_TAG_ASSET_MANAGER),
        *image1 = memalloc(sizeof(Image), MEMORY_TAG_ASSET_MANAGER);
    image0->width = image0->height = 1;
    image1->width = image1->height = 1;
    image0->data = memalloc(sizeof(uint32_t), MEMORY_TAG_ASSET_MANAGER);
    image1->data = memalloc(sizeof(uint32_t), MEMORY_TAG_ASSET_MANAGER);
    memset(image0->data, 0, sizeof(uint32_t));
    memset(image1->data, 255, sizeof(uint32_t));
    hashmap_put(images, hash_string("__baseColor0"), (uint64_t)image0);
    hashmap_put(images, hash_string("__baseColor255"), (uint64_t)image1);
}

Material* create_default_material(HashMap* images) {
    Material* ret = memalloc(sizeof(Material), MEMORY_TAG_ASSET_MANAGER);
    ret->baseColor.image = (Image*)hashmap_get(images, hash_string("__baseColor255"));
    ret->normal.image = (Image*)hashmap_get(images, hash_string("__baseColor0"));
    ret->metallicRoughness.image = (Image*)hashmap_get(images, hash_string("__baseColor0"));
    ret->pipelineType = PIPELINE_TYPE_DEFAULT;
    ret->meshes = darray_create_memoryTag(Mesh, MEMORY_TAG_ASSET_MANAGER);
    return ret;
}

void calculate_model_AABB(Model* model) {
    AABB aabb = {
        .min = {{INFINITY, INFINITY, INFINITY}},
        .max = {{-INFINITY, -INFINITY, -INFINITY}}
    };
    Material* material;
    hashmap_foreach(model->materials, material) {
        int meshCount = darray_get_length(material->meshes);
        for (int i = 0; i < meshCount; i++) {
            aabb.min = vec3_min(aabb.min, material->meshes[i].collider.aabb.min);
            aabb.max = vec3_max(aabb.max, material->meshes[i].collider.aabb.max);
        }
    }
    model->collider.aabb = aabb;
}

void calculate_mesh_AABB(Mesh* mesh) {
    AABB aabb = {
        .min = {{INFINITY, INFINITY, INFINITY}},
        .max = {{-INFINITY, -INFINITY, -INFINITY}}
    };
    int vertexCount = darray_get_length(mesh->vertices);
    for (int i = 0; i < vertexCount; i++) {
        vec3 position = mesh->vertices[i].position;
        aabb.min = vec3_min(aabb.min, position);
        aabb.max = vec3_max(aabb.max, position);
    }    
    mesh->collider.aabb.min = aabb.min;
    mesh->collider.aabb.max = aabb.max;
}