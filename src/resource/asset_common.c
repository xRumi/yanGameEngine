#include "asset_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

void imagesPutDefaultImages(HashMap* images) {
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

Image* imageLoadFromPath(const char* path) {
    (void)stbi__mul2shorts_valid;
    (void)stbi__addints_valid;
    int width = 0, height = 0, channel = 0;
    stbi_uc* pixels = stbi_load(path, &width, &height, &channel, STBI_rgb_alpha);
    if (!pixels) {
        WARN("Failed to load image, skipping");
        return NULL;
    }
    Image* image = memalloc(sizeof(Image), MEMORY_TAG_ASSET_MANAGER);
    image->height = height;
    image->width = width;
    image->data = pixels;
    return image;
}

Material* materialFromDefaultImages(HashMap* images) {
    Material* ret = memalloc(sizeof(Material), MEMORY_TAG_ASSET_MANAGER);
    ret->baseColor.image = (Image*)hashmap_get(images, hash_string("__baseColor255"));
    ret->normal.image = (Image*)hashmap_get(images, hash_string("__baseColor0"));
    ret->metallicRoughness.image = (Image*)hashmap_get(images, hash_string("__baseColor0"));
    ret->pipelineType = PIPELINE_TYPE_DEFAULT;
    return ret;
}

mat4 mat4FromTransform(Transform transform) {
    mat4 translation = mat4_translation_vec3(transform.translation);
    mat4 rotation = mat4_mul(mat4_rotation_z(transform.rotation.z), mat4_mul(mat4_rotation_x(transform.rotation.y), mat4_rotation_y(transform.rotation.x)));
    mat4 scale = mat4_scale_vec3(transform.scale);
    return mat4_mul(translation, mat4_mul(rotation, scale));
}

void updateModelColliderHalfDimensions(Model* model) {
    vec3 halfDimensions = {};
    Material* material;
    hashmap_foreach(model->materials, material) {
        int meshCount = darray_get_length(model->meshes);
        for (int i = 0; i < meshCount; i++) {
            halfDimensions = vec3_max(halfDimensions, model->meshes[i].collider.halfDimensions);
        }
    }
    model->collider.halfDimensions = halfDimensions;
}

void updateMeshColliderHalfDimensions(Mesh* mesh) {
    vec3 min = {{INFINITY, INFINITY, INFINITY}};
    vec3 max = {{-INFINITY, -INFINITY, -INFINITY}};
    int vertexCount = darray_get_length(mesh->vertices);
    for (int i = 0; i < vertexCount; i++) {
        vec3 position = mesh->vertices[i].position;
        min = vec3_min(min, position);
        max = vec3_max(max, position);
    }
    mesh->collider.halfDimensions = vec3_scale(vec3_sub(max, min), 0.5);
}