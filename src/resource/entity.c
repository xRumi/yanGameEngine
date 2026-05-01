#include "asset_manager.h"

uint64_t UniqueEntityId = 1;

Entity* entityCreate(Model* model) {
    Entity* entity = memalloc(sizeof(Entity), MEMORY_TAG_ENTITY);
    entity->id = UniqueEntityId++;
    entity->model = model;
    entity->nodeAnimations = entityCreateNodeAnimations(model);
    entity->isHidden = true;
    entity->transform.scale = (vec3){{1, 1, 1}};
    entity->modelMatrix.buffers[0] = mat4_identity();
    entity->modelMatrix.buffers[1] = mat4_identity();
    entity->collider = model->collider;
    entity->collider.center = &entity->transform.translation;
    entity->timeManager = timeManagerStart();
    return entity;
}

void entitySetHidden(Entity* entity, bool isHidden) {
    entity->isHidden = isHidden;
}

void entityTransformSetTranslation(Entity* entity, vec3 translation) {
    entity->transform.translation = translation;
}
void entityTransformApply(Entity* entity) {
    mat4 model = mat4FromTransform(entity->transform);
    atomicMatrixSetMatrix(&entity->modelMatrix, model);
}
void entityTransformReset(Entity* entity) {
    entity->transform.translation = (vec3){{}};
    entity->transform.rotation = (vec3){{}};
    entity->transform.scale = (vec3){{1, 1, 1}};
    entityTransformApply(entity);
}
mat4 entityGetModelMatrix(Entity* entity) {
    return atomicMatrixGetMatrix(&entity->modelMatrix);
}
void entityPhysicsBodyAddForce(Entity* entity, vec3 force) {
    if (entity->physicsBody) physicsBodyAddForce(entity->physicsBody, force);
}
HashMap* entityCreateNodeAnimations(Model* model) {
    HashMap* nodeAnimations = hashmap_create(20);
    Node* node;
    hashmap_foreach(model->nodes, node) {
        if (node->isAnimated) {
            NodeAnimation* nodeAnimation = memalloc(sizeof(NodeAnimation), MEMORY_TAG_ASSET_MANAGER);
            nodeAnimation->node = node;
            nodeAnimation->propagration = mat4_identity();
            atomicMatrixSetMatrix(&nodeAnimation->matrix, node->matrix);
            hashmap_put(nodeAnimations, (uint64_t)node, (uint64_t)nodeAnimation);
        }
    }
    return nodeAnimations;
}