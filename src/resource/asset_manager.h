#pragma once
#include "asset_types.h"
#include "emath.h"
#include "defines.h"
#include "math_types.h"
#include "darray.h"

Entity* entityCreate(Model* model);
void entityDestroy(Entity* entity);
void entityTransformSetTranslation(Entity* entity, vec3 a);
void entityTransformApply(Entity* entity);
void entityTransformReset(Entity* entity);
mat4 entityModelMatrixGet(Entity* entity);

Scene* sceneCreate();
void sceneDestroy(Scene* scene);
void sceneAddEntity(Scene* scene, Entity* entity);
void sceneRemoveEntity(Scene* scene, Entity* entity);
void sceneEntityTransformApply(Scene* scene);
PointLight* sceneAddPointLight(Scene* scene);
void sceneRemovePointLight(Scene* scene, PointLight* light);
DirectionalLight* sceneAddDirectionalLight(Scene* scene);
void sceneRemoveDirectionalLight(Scene* scene, DirectionalLight* light);

void sceneEntityPhysicsBodyCreate(Scene* scene, Entity* entity);

Model* assetLoadGLTF(const char* gltf_dir, const char* gltf_file);
Model* assetGenerateUVSphere(int slices, int stacks, float radius);
void load_default_images(HashMap* images);
Material* create_default_material(HashMap* images);