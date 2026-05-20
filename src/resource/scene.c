#include "asset_manager.h"


Scene* sceneCreate() {
    Scene* scene = memalloc(sizeof(Scene), MEMORY_TAG_ASSET_MANAGER);
    scene->camera.sensitivity = 1;
    scene->entities = hashmap_create(1000);
    scene->physicsEngine = physicsEngineCreate();
    return scene;
}
void sceneDestroy(Scene* scene);

Entity* sceneCreateEntity(Scene* scene, Model* model) {
    Entity* entity = entityCreate(model);
    sceneAddEntity(scene, entity);
    return entity;
}
void sceneAddEntity(Scene* scene, Entity* entity) {
    entity->scene = scene;
    hashmap_put(scene->entities, entity->id, (uint64_t)entity);
}
void sceneDestoryEntity(Scene* scene, Entity* entity) {
    hashmap_remove(scene->entities, entity->id);
    WARN("Entity destory not implemented; todo");
}
void sceneEntityApplyTransform(Scene* scene) {
    Entity* entity;
    hashmap_foreach(scene->entities, entity) {
        entity->generation++;
        timeManagerUpdate(&entity->timeManager);
        entityNodeAnimationApply(entity);
        entityTransformApply(entity);
    }
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
void sceneCameraSetPosition(Scene* scene, vec3 position) {
    atomicVec3SetVec3(&scene->camera.position, position);
}