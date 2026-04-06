#include "asset_manager.h"

uint64_t assetManagerEntityId = 1;

Entity* entityCreate(Model* model) {
    Entity* entity = memalloc(sizeof(Entity), MEMORY_TAG_ENTITY);
    entity->id = assetManagerEntityId++;
    entity->model = model;
    entity->transform = mat4_identity();
    return entity;
}
void entityApplyTransform(Entity* entity, mat4 transform) {
    entity->transform = mat4_mul(entity->transform, transform);
}
void entityResetTransform(Entity* entity) {
    entity->transform = mat4_identity();
}

Scene* sceneCreate() {
    Scene* scene = memalloc(sizeof(Scene), MEMORY_TAG_ASSSET_MANAGER);
    scene->camera.sensitivity = 200;
    scene->entities = hashmap_create(1000);
    return scene;
}
void sceneDestroy(Scene* scene);
void sceneAddEntity(Scene* scene, Entity* entity) {
    hashmap_put(scene->entities, entity->id, (uint64_t)entity);
}
void sceneRemoveEntity(Scene* scene, Entity* entity) {
    hashmap_remove(scene->entities, entity->id);
}

PointLight* sceneAddPointLight(Scene* scene) {
    if (scene->lightUBO.pointLightCount >= POINT_LIGHT_MAX_COUNT) return NULL;
    return &scene->lightUBO.pointLights[(int)scene->lightUBO.pointLightCount++];
}
void sceneRemovePointLight(Scene* scene, PointLight* light) {
    if (scene->lightUBO.pointLightCount <= 0) return;
    for (int i = 0; i < POINT_LIGHT_MAX_COUNT; i++)
        if (light == &scene->lightUBO.pointLights[i]) {
            scene->lightUBO.pointLights[i] = scene->lightUBO.pointLights[(int)scene->lightUBO.pointLightCount - 1];
            scene->lightUBO.pointLightCount--;
            break;
        }
}
DirectionalLight* sceneAddDirectionalLight(Scene* scene) {
    if (scene->lightUBO.directionalLightCount >= POINT_LIGHT_MAX_COUNT) return NULL;
    return &scene->lightUBO.directionalLights[(int)scene->lightUBO.directionalLightCount++];
}
void sceneRemoveDirectionalLight(Scene* scene, DirectionalLight* light) {
    if (scene->lightUBO.directionalLightCount <= 0) return;
    for (int i = 0; i < DIRECTIONAL_LIGHT_MAX_COUNT; i++)
        if (light == &scene->lightUBO.directionalLights[i]) {
            scene->lightUBO.directionalLights[i] = scene->lightUBO.directionalLights[(int)scene->lightUBO.directionalLightCount - 1];
            scene->lightUBO.directionalLightCount--;
            break;
        }
}