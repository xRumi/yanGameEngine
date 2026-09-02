#include "asset_manager.h"


Scene* sceneCreate() {
    Scene* scene = memalloc(sizeof(Scene), MEMORY_TAG_ASSET_MANAGER);
    scene->camera.sensitivity = 1;
    scene->entities = hashmap_create(1000);
    scene->light.lightToEntity = hashmap_create(100);
    scene->physicsEngine = physicsEngineCreate();
    return scene;
}
void sceneDestroy(Scene* scene);

Entity* sceneCreateEntity(Scene* scene, Model* model) {
    Entity* entity = entityCreate(model, false);
    sceneAddEntity(scene, entity);
    return entity;
}
Entity* sceneCreateEntityHidden(Scene* scene, Model* model) {
    Entity* entity = entityCreate(model, true);
    sceneAddEntity(scene, entity);
    return entity;
}

void sceneAddEntity(Scene* scene, Entity* entity) {
    entity->scene = scene;
    hashmap_put(scene->entities, entity->id, (uint64_t)entity);
}
void sceneRemoveEntity(Scene* scene, Entity* entity) {
    hashmap_remove(scene->entities, entity->id);
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

Light* sceneCreatePointLight(Scene* scene, Model* lightModel, PointLight pointLight) {
    if (scene->light.pointLightCount >= POINT_LIGHT_MAX_COUNT) return NULL;
    Entity* lightEntity = sceneCreateEntity(scene, lightModel);
    lightEntity->isLightSource = true;
    lightEntity->transform.translation = vec3_from_vec4(pointLight.position);
    Light* light = memalloc(sizeof(Light), MEMORY_TAG_ASSET_MANAGER);
    light->type = LIGHT_TYPE_POINT;
    light->data = memalloc(sizeof(PointLight), MEMORY_TAG_ASSET_MANAGER);
    *(PointLight*)light->data = pointLight;
    hashmap_put(scene->light.lightToEntity, (uint64_t)light, (uint64_t)lightEntity);
    return light;
}
Entity* scenePointLightGetEntity(Scene* scene, Light* light) {
    return (Entity*)hashmap_get(scene->light.lightToEntity, (uint64_t)light);
}
void sceneDestroyLight(Scene* scene, Light* light) {
    if (scene->light.pointLightCount <= 0) return;
    Entity* entity = (Entity*)hashmap_get(scene->light.lightToEntity, (uint64_t)light);
    if (entity) {
        sceneRemoveEntity(scene, entity);
        entityDestroy(entity);
    }
    switch (light->type) {
        case LIGHT_TYPE_POINT: 
            memfree(light->data, sizeof(PointLight), MEMORY_TAG_ASSET_MANAGER);
            break;
        case LIGHT_TYPE_DIRECTIONAL:
            memfree(light->data, sizeof(DirectionalLight), MEMORY_TAG_ASSET_MANAGER);
            break;
    }
    memfree(light, sizeof(Light), MEMORY_TAG_ASSET_MANAGER);
    scene->light.pointLightCount--;
}
Light* sceneCreateDirectionalLight(Scene* scene, DirectionalLight directionalLight) {
    if (scene->light.pointLightCount >= POINT_LIGHT_MAX_COUNT) return NULL;
    Light* light = memalloc(sizeof(Light), MEMORY_TAG_ASSET_MANAGER);
    light->type = LIGHT_TYPE_DIRECTIONAL;
    light->data = memalloc(sizeof(DirectionalLight), MEMORY_TAG_ASSET_MANAGER);
    *(DirectionalLight*)light->data = directionalLight;
    hashmap_put(scene->light.lightToEntity, (uint64_t)light, -1);
    return light;
}
void sceneCameraSetPosition(Scene* scene, vec3 position) {
    atomicVec3SetVec3(&scene->camera.position, position);
}