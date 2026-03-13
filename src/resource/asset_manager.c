#include "asset_manager.h"

Entity createEntity(Model* model) {
    Entity entity = {};
    entity.modelRef = model;
    entity.transform = mat4_identity();
    return entity;
}
