#pragma once
#include "asset_types.h"
#include "emath.h"
#include "defines.h"
#include "math_types.h"
#include "darray.h"

Entity* entityCreate(Model* model);
void entityDestroy(Entity* entity);
void entityApplyTransform(Entity* entity, mat4 transform);
void entityResetTransform(Entity* entity);

Scene* sceneCreate();
void sceneDestroy(Scene* scene);
void sceneAddEntity(Scene* scene, Entity* entity);
void sceneRemoveEntity(Scene* scene, Entity* entity);
PointLight* sceneAddPointLight(Scene* scene);
void sceneRemovePointLight(Scene* scene, PointLight* light);
DirectionalLight* sceneAddDirectionalLight(Scene* scene);
void sceneRemoveDirectionalLight(Scene* scene, DirectionalLight* light);

Model* modelCreate(const char* gltf_dir, const char* gltf_file);