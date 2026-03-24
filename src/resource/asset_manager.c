#include "asset_manager.h"

uint64_t assetManagerEntityId = 1;

Entity* entityCreate(Model* model) {
    Entity* entity = memalloc(sizeof(Entity), MEMORY_TAG_ENTITY);
    entity->id = assetManagerEntityId++;
    entity->modelRef = model;
    entity->transform = blenderToClipSpace();
    return entity;
}
void entityApplyTransform(Entity* entity, mat4 transform) {
    entity->transform = mat4_mul(entity->transform, transform);
}
void entityResetTransform(Entity* entity) {
    entity->transform = blenderToClipSpace();
}
