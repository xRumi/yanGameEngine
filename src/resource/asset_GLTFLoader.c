#include "asset_manager.h"
#include "asset_types.h"
#include "hashMap.h"
#include "utils.h"
#include <errno.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"

struct {
} internalStateGLTFLoader;

uint32_t* loadGLTFMeshIndices(const cgltf_accessor* accessor) {
    if (!accessor || accessor->type != cgltf_type_scalar) return NULL;
    unsigned long count = accessor->count,
        offset = accessor->offset,
        stride = accessor->stride;
    uint8_t* data = (uint8_t*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset;
    uint32_t* indices = darray_create_resized_memoryTag(uint32_t, count, MEMORY_TAG_ASSET_MANAGER);
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

Vertex* loadGLTFMeshVertices(const cgltf_attribute* attributes, uint32_t attributeCount) {
    if (!attributes) return NULL;
    uint32_t vertexCount = attributes[0].data->count;
    Vertex* vertices = darray_create_resized_memoryTag(Vertex, vertexCount, MEMORY_TAG_ASSET_MANAGER);
    for (int i = 0; i < vertexCount; i++) {
        vertices[i] = (Vertex){
            .position = {{0, 0, 0}},
            .color = {{1, 1, 1, 1}},
            .texCoord = {{0, 0}},
            .normal = {{0, 0, 1}},
            .tangent = {{1, 0, 0, 1}},
        };
        vertices[i].color = (vec4){{
            (rand() % 256) / 256.0,
            (rand() % 256) / 256.0,
            (rand() % 256) / 256.0,
            (rand() % 256) / 256.0,
        }};
    }
    for (int i = 0; i < attributeCount; i++) {
        cgltf_attribute_type attributeType = attributes[i].type;
        cgltf_accessor* accessor = attributes[i].data;

        uint8_t* data = (uint8_t*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset;

        uint64_t dataCount = accessor->count,
            offset = accessor->offset,
            stride = accessor->stride;

        bool skipAttribute = false;

        for (int j = 0; j < dataCount; j++) {
            switch (attributeType) {
                case cgltf_attribute_type_position: {
                    vec3 position = {};
                    memcpy(&position, data + offset + stride * j, sizeof(vec3));
                    vertices[j].position = position;
                    break;
                }
                case cgltf_attribute_type_color: {
                    vec4 color = {{0, 0, 0, 1}};
                    memcpy(&color, data + offset + stride * j, cgltf_calc_size(accessor->type, accessor->component_type));
                    vertices[j].color = color;
                    break;
                }
                case cgltf_attribute_type_texcoord: {
                    vec2 texCoord = {};
                    memcpy(&texCoord, data + offset + stride * j, sizeof(vec2));
                    vertices[j].texCoord = texCoord;
                    break;
                }
                case cgltf_attribute_type_normal: {
                    vec3 normal = {};
                    memcpy(&normal, data + offset + stride * j, sizeof(vec3));
                    vertices[j].normal = normal;
                    break;
                }
                case cgltf_attribute_type_tangent: {
                    vec4 tangent = {{0, 0, 0, 1}};
                    memcpy(&tangent, data + offset + stride * j, cgltf_calc_size(accessor->type, accessor->component_type));
                    vertices[j].tangent = tangent;
                    break;
                }
                case cgltf_attribute_type_joints: {
                    vec4 joints = {};
                    if (accessor->component_type == cgltf_component_type_r_16u) {
                        uint16_t* tempJoints = (uint16_t*)(data + offset + stride * j);
                        joints.ele[0] = tempJoints[0];
                        joints.ele[1] = tempJoints[1];
                        joints.ele[2] = tempJoints[2];
                        joints.ele[3] = tempJoints[3];
                    } else memcpy(&joints, data + offset + stride * j, cgltf_calc_size(accessor->type, accessor->component_type));
                    vertices[j].joints = joints;
                    break;
                }
                case cgltf_attribute_type_weights: {
                    vec4 weights = {{0, 0, 0, 0}};
                    memcpy(&weights, data + offset + stride * j, cgltf_calc_size(accessor->type, accessor->component_type));
                    vertices[j].weights = weights;
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

float findNodeAnimationTime(Node* node) {
    float animationTime = node->animationSampler.animationTime;
    Node** childRef;
    darray_foreach(node->child, childRef) {
        Node* child = *childRef;
        animationTime = MAX(animationTime, findNodeAnimationTime(child));
    }
    return animationTime;
}
void setNodeAnimationTime(Node* node, float animationTime) {
    node->animationSampler.animationTime = animationTime;
    Node** childRef;
    darray_foreach(node->child, childRef) {
        Node* child = *childRef;
        setNodeAnimationTime(child, animationTime);
    }
}
void syncNodeAnimationTime(HashMap* nodes) {
    Node* node;
    hashmap_foreach(nodes, node) {
        if (!node->isAnimated) continue;
        float animationTime = findNodeAnimationTime(node);
        setNodeAnimationTime(node, animationTime);
    }
}

HashMap* loadGLTFNodes(cgltf_data* gltf_data) {
    HashMap* nodes = hashmap_create(gltf_data->nodes_count);
    for (int i = 0; i < gltf_data->nodes_count; i++) {
        Node* node = memalloc(sizeof(Node), MEMORY_TAG_ASSET_MANAGER);
        node->child = darray_create_resized_memoryTag(Node*, gltf_data->nodes[i].children_count, MEMORY_TAG_ASSET_MANAGER);
        cgltf_node_transform_world(&gltf_data->nodes[i], node->matrix.ele);
        hashmap_put(nodes, (uint64_t)&gltf_data->nodes[i], (uint64_t)node);
    }
    for (int i = 0; i < gltf_data->nodes_count; i++) {
        Node* node = (Node*)hashmap_get(nodes, (uint64_t)&gltf_data->nodes[i]);
        for (int j = 0; j < gltf_data->nodes[i].children_count; j++) {
            Node* child = (Node*)hashmap_get(nodes, (uint64_t)gltf_data->nodes[i].children[j]);
            child->parent = node;
            node->child[j] = child;
        }
        if (gltf_data->nodes[i].skin) {
            int jointCount = gltf_data->nodes[i].skin->joints_count;
            node->joints = darray_create_resized_memoryTag(Node*, jointCount, MEMORY_TAG_ASSET_MANAGER);
            for (int j = 0; j < jointCount; j++) {
                Node* joint = (Node*)hashmap_get(nodes, (uint64_t)gltf_data->nodes[i].skin->joints[j]);
                cgltf_accessor_read_float(gltf_data->nodes[i].skin->inverse_bind_matrices, j, joint->inverseBindMatrix.ele, 16);
                node->joints[j] = joint;
            }

        }
    }
    for (int i = 0; i < gltf_data->animations_count; i++) {
        int channelCount = gltf_data->animations[i].channels_count;
        for (int j = 0; j < channelCount; j++) {
            cgltf_animation_channel* channel = &gltf_data->animations[i].channels[j];
            cgltf_animation_sampler* sampler = channel->sampler;
            Node* node = (Node*)hashmap_get(nodes, (uint64_t)channel->target_node);
            node->isAnimated = true;
            Node** childRef;
            darray_foreach(node->child, childRef) {
                Node* child = *childRef;
                child->isAnimated = true;
            }
            switch (channel->target_path) {
                case cgltf_animation_path_type_invalid:
                case cgltf_animation_path_type_translation: {
                    node->animationSampler.translation.input = darray_create_resized_memoryTag(float, sampler->input->count, MEMORY_TAG_ASSET_MANAGER);
                    node->animationSampler.translation.output = darray_create_resized_memoryTag(vec3, sampler->output->count, MEMORY_TAG_ASSET_MANAGER);
                    for (int k = 0; k < sampler->input->count; k++) {
                        float time = 0;
                        vec3 translation = {};
                        memcpy(&time, sampler->input->buffer_view->buffer->data + sampler->input->offset + sampler->input->buffer_view->offset + sampler->input->stride * k, sizeof(float));
                        memcpy(&translation, sampler->output->buffer_view->buffer->data + sampler->output->offset + sampler->output->buffer_view->offset + sampler->output->stride * k, sizeof(vec3));
                        node->animationSampler.translation.input[k] = time;
                        node->animationSampler.translation.output[k] = translation;
                        node->animationSampler.animationTime = MAX(node->animationSampler.animationTime, time);
                    }
                    break;
                }
                case cgltf_animation_path_type_rotation: {
                    node->animationSampler.rotation.input = darray_create_resized_memoryTag(float, sampler->input->count, MEMORY_TAG_ASSET_MANAGER);
                    node->animationSampler.rotation.output = darray_create_resized_memoryTag(vec4, sampler->output->count, MEMORY_TAG_ASSET_MANAGER);
                    for (int k = 0; k < sampler->input->count; k++) {
                        float time = 0;
                        vec4 rotation = {};
                        memcpy(&time, sampler->input->buffer_view->buffer->data + sampler->input->offset + sampler->input->buffer_view->offset + sampler->input->stride * k, sizeof(float));
                        memcpy(&rotation, sampler->output->buffer_view->buffer->data + sampler->output->offset + sampler->output->buffer_view->offset + sampler->output->stride * k, sizeof(vec4));
                        node->animationSampler.rotation.input[k] = time;
                        node->animationSampler.rotation.output[k] = rotation;
                        node->animationSampler.animationTime = MAX(node->animationSampler.animationTime, time);
                    }
                    break;
                }
                case cgltf_animation_path_type_scale: {
                    node->animationSampler.scale.input = darray_create_resized_memoryTag(float, sampler->input->count, MEMORY_TAG_ASSET_MANAGER);
                    node->animationSampler.scale.output = darray_create_resized_memoryTag(vec3, sampler->output->count, MEMORY_TAG_ASSET_MANAGER);
                    for (int k = 0; k < sampler->input->count; k++) {
                        float time = 0;
                        vec3 scale = {};
                        memcpy(&time, sampler->input->buffer_view->buffer->data + sampler->input->offset + sampler->input->buffer_view->offset + sampler->input->stride * k, sizeof(float));
                        memcpy(&scale, sampler->output->buffer_view->buffer->data + sampler->output->offset + sampler->output->buffer_view->offset + sampler->output->stride * k, sizeof(vec3));
                        node->animationSampler.scale.input[k] = time;
                        node->animationSampler.scale.output[k] = scale;
                        node->animationSampler.animationTime = MAX(node->animationSampler.animationTime, time);
                    }
                    break;
                }
                case cgltf_animation_path_type_weights:
                case cgltf_animation_path_type_max_enum:
                default: WARN("Unknown animation path type");
                break;
            }
        }
    }

    // sync animationTime of all the connected nodes
    syncNodeAnimationTime(nodes);

    return nodes;
}

void updateMeshColliderHalfDimensionsIfProvided(Mesh* mesh, const cgltf_attribute* attributes, uint32_t attributeCount) {
    for (int i = 0; i < attributeCount; i++)
        if (attributes[i].type == cgltf_attribute_type_position) {
            cgltf_accessor* accessor = attributes[i].data;
            if (accessor->has_max && accessor->has_min) {
                vec3 min = (vec3){{accessor->min[0], accessor->min[1], accessor->min[2]}};
                vec3 max = (vec3){{accessor->max[0], accessor->max[1], accessor->max[2]}};
                mesh->collider.halfDimensions = vec3_scale(vec3_sub(max, min), 0.5);
                return;
            }
        }
    TRACE("AABB not provided, generating overself");
    updateMeshColliderHalfDimensions(mesh);
}

HashMap* loadGLTFImages(const char* gltf_dir, cgltf_image* images, uint32_t imageCount) {
    HashMap* imageHashMap = hashmap_create(imageCount * 10 + 1);
    imagesPutDefaultImages(imageHashMap);
    char path[256];
    for (uint32_t i = 0; i < imageCount; i++) {
        if (!images[i].uri) {
            WARN("Found image with null uri, skipping");
            continue;
        }
        snprintf(path, 256, "%s/%s", gltf_dir, images[i].uri);
        Image* image = imageLoadFromPath(path);
        if (!image) continue;
        hashmap_put(imageHashMap, hash_string(images[i].uri), (uint64_t)image);
    }
    return imageHashMap;
}

HashMap* loadGLTFMaterials(cgltf_material* gltf_material, uint32_t materialCount, HashMap* images) {
    HashMap* materials = hashmap_create(materialCount * 10 + 1);
    hashmap_put(materials, 0, (uint64_t)materialFromDefaultImages(images));
    for (int i = 0; i < materialCount; i++) {
        Material* material = materialFromDefaultImages(images);
        if (gltf_material[i].pbr_metallic_roughness.base_color_texture.texture)
            material->baseColor.image = (Image*)hashmap_get(images, hash_string(gltf_material[i].pbr_metallic_roughness.base_color_texture.texture->image->uri));
        if (gltf_material[i].normal_texture.texture)
            material->normal.image = (Image*)hashmap_get(images, hash_string(gltf_material[i].normal_texture.texture->image->uri));
        if (gltf_material[i].pbr_metallic_roughness.metallic_roughness_texture.texture)
            material->metallicRoughness.image = (Image*)hashmap_get(images, hash_string(gltf_material[i].pbr_metallic_roughness.metallic_roughness_texture.texture->image->uri));
        hashmap_put(materials, (uint64_t)&gltf_material[i], (uint64_t)material);
    }
    return materials;
}

Model* assetLoadGLTF(const char* gltf_dir, const char* gltf_file) {
    char* traceStr = darray_create(char);
    stringBuilderConcat(&traceStr, ANSI_COLOR_YELLOW "\b[GLTF_LOADER]" ANSI_RESET_ALL " Loading model \"%s\"\n", gltf_file);

    cgltf_options options = {};
    cgltf_data* gltf_data;
    char gltf_path[256];
    snprintf(gltf_path, 256, "%s/%s", gltf_dir, gltf_file);
    if (cgltf_parse_file(&options, gltf_path, &gltf_data) != cgltf_result_success) {
        WARN("Failed to load model \"%s\": %s", gltf_path, strerror(errno));
        return NULL;
    }
    if (cgltf_load_buffers(&options, gltf_data, gltf_path) != cgltf_result_success) {
        WARN("Failed to load model buffers \"%s\": %s", gltf_path, strerror(errno));
        return NULL;
    }

    Model* model = memalloc(sizeof(Model), MEMORY_TAG_ASSET_MANAGER);
    model->name = gltf_file;
    model->images = loadGLTFImages(gltf_dir, gltf_data->images, gltf_data->images_count);
    model->materials = loadGLTFMaterials(gltf_data->materials, gltf_data->materials_count, model->images);
    model->meshes = darray_create_resized_memoryTag(Mesh, gltf_data->meshes_count, MEMORY_TAG_ASSET_MANAGER);
    model->nodes = loadGLTFNodes(gltf_data);

    stringBuilderConcat(&traceStr, "Image: %d\n", gltf_data->images_count);
    stringBuilderConcat(&traceStr, "Material: %d", gltf_data->materials_count);

    for (int i = 0; i < gltf_data->meshes_count; i++) {

        int primitives_count = gltf_data->meshes[i].primitives_count;
        for (int j = 0; j < primitives_count; j++) {
            cgltf_primitive* primitive = &gltf_data->meshes[i].primitives[j];

            switch (primitive->type) {
                case cgltf_primitive_type_triangles: {
                    if (!hashmap_has(model->materials, (uint64_t)primitive->material)) {
                        WARN("Material not found for a mesh, skipping");
                        break;
                    }

                    Mesh mesh = {};

                    stringBuilderConcat(&traceStr, "\nSub-Mesh %d/%d:\n", i + 1, gltf_data->meshes_count);
                    stringBuilderConcat(&traceStr, "Indices    = %d\n", primitive->indices->count);
                    mesh.indices = loadGLTFMeshIndices(primitive->indices);
                    if (mesh.indices == NULL) FATAL("[%s] Failed to load indices", gltf_path);

                    stringBuilderConcat(&traceStr, "Vertices   = %d\n", primitive->attributes[0].data->count);
                    mesh.vertices = loadGLTFMeshVertices(primitive->attributes, primitive->attributes_count);
                    if (mesh.vertices == NULL) FATAL("[%s] Failed to load vertices", gltf_path);
                    updateMeshColliderHalfDimensionsIfProvided(&mesh, primitive->attributes, primitive->attributes_count);

                    mesh.material = (Material*)hashmap_get(model->materials, (uint64_t)primitive->material);
                    if (model->meshes[i].vertices) {
                        Mesh* tail = &model->meshes[i];
                        while (tail->next) tail = tail->next;
                        tail->next = memalloc(sizeof(Mesh), MEMORY_TAG_ASSET_MANAGER);
                        memcpy(tail->next, &mesh, sizeof(Mesh));
                    } else model->meshes[i] = mesh;

                    stringBuilderConcat(&traceStr, "Attributes = ");
                    for (int i = 0; i < primitive->attributes_count; i++)
                        stringBuilderConcat(&traceStr, "%s, ", primitive->attributes[i].name);
                    stringBuilderConcat(&traceStr, "\n");
                    break;
                }
                default: {
                    WARN("[%s] Unknown primitive provided", gltf_path);
                }
            }
        }

        for (int k = 0; k < gltf_data->nodes_count; k++) {
            if (gltf_data->nodes[k].mesh == &gltf_data->meshes[i]) {
                Node* node = (Node*)hashmap_get(model->nodes, (uint64_t)&gltf_data->nodes[k]);
                node->mesh = &model->meshes[i];
            }
        }
    }
    updateModelColliderHalfDimensions(model);
    cgltf_free(gltf_data);
    stringBuilderConcat(&traceStr, "-----------------------------------------------------------", gltf_file);
    TRACE(traceStr);
    darray_destroy(traceStr);
    return model;
}