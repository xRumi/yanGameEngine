#include "asset_manager.h"

uint64_t assetManagerEntityId = 1;

void entitySetHidden(Entity* entity, bool isHidden) {
    entity->isHidden = isHidden;
}

void entityTransformSetTranslation(Entity* entity, vec3 translation) {
    entity->transform.translation = translation;
}
void entityTransformApply(Entity* entity) {
    mat4 translation = mat4_translation_vec3(entity->transform.translation);
    mat4 rotation = mat4_mul(mat4_rotation_z(entity->transform.rotation.z), mat4_mul(mat4_rotation_x(entity->transform.rotation.y), mat4_rotation_y(entity->transform.rotation.x)));
    mat4 scale = mat4_scale_vec3(entity->transform.scale);
    mat4 model = mat4_mul(translation, mat4_mul(rotation, scale));

    entity->collider.aabb.max = mat4_mul_vec3(model, entity->model->collider.aabb.max);
    entity->collider.aabb.min = mat4_mul_vec3(model, entity->model->collider.aabb.min);
    entity->collider.boundingSphere.center = mat4_mul_vec3(model, entity->model->collider.boundingSphere.center);

    entity->modelMatrix.shouldYield = true;
    while (atomic_flag_test_and_set(&entity->modelMatrix.locked) != 0);
    entity->modelMatrix.buffers[1] = model;
    atomic_flag_clear(&entity->modelMatrix.locked);
    entity->modelMatrix.shouldYield = false;
}
void entityTransformReset(Entity* entity) {
    entity->transform.translation = (vec3){{}};
    entity->transform.rotation = (vec3){{}};
    entity->transform.scale = (vec3){{1, 1, 1}};
    entityTransformApply(entity);
}
mat4 entityGetModelMatrix(Entity* entity) {
    if (!entity->modelMatrix.shouldYield && atomic_flag_test_and_set(&entity->modelMatrix.locked) == 0) {
        mat4 model = entity->modelMatrix.buffers[1];
        atomic_flag_clear(&entity->modelMatrix.locked);
        entity->modelMatrix.buffers[0] = model;
    }
    return entity->modelMatrix.buffers[0];
}
void entityPhysicsBodyAddForce(Entity* entity, vec3 force) {
    if (entity->physicsBody) physicsBodyAddForce(entity->physicsBody, force);
}

Scene* sceneCreate() {
    Scene* scene = memalloc(sizeof(Scene), MEMORY_TAG_ASSET_MANAGER);
    scene->camera.sensitivity = 1;
    scene->entities = hashmap_create(1000);
    scene->physicsEngine = physicsEngineCreate();
    return scene;
}
void sceneDestroy(Scene* scene);

Entity* sceneCreateEntity(Scene* scene, Model* model) {
    Entity* entity = memalloc(sizeof(Entity), MEMORY_TAG_ENTITY);
    entity->id = assetManagerEntityId++;
    entity->model = model;
    entity->collider = model->collider;
    entity->isHidden = true;
    entity->transform.scale = (vec3){{1, 1, 1}};
    entity->modelMatrix.buffers[0] = mat4_identity();
    entity->modelMatrix.buffers[1] = mat4_identity();
    hashmap_put(scene->entities, entity->id, (uint64_t)entity);
    return entity;
}
void sceneDestoryEntity(Scene* scene, Entity* entity) {
    hashmap_remove(scene->entities, entity->id);
    WARN("Entity destory not implemented; todo");
}
void sceneEntityApplyTransform(Scene* scene) {
    Entity* entity;
    hashmap_foreach(scene->entities, entity) {
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

void sceneEntityCreatePhysicsBody(Scene* scene, Entity* entity) {
    PhysicsBody* physicsBody = memalloc(sizeof(PhysicsBody), MEMORY_TAG_PHYSICS);
    physicsBody->mass = 1;
    physicsBody->inverseMass = 1.0 / physicsBody->mass;
    physicsBody->collider = &entity->collider;
    darray_push(scene->physicsEngine->bodies, physicsBody);
    entity->physicsBody = physicsBody;
    physicsBody->transform = &entity->transform;
}