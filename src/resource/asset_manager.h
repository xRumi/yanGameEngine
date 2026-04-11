#pragma once
#include "asset_types.h"
#include "emath.h"
#include "defines.h"
#include "math_types.h"
#include "darray.h"

void entityTransformSetTranslation(Entity* entity, vec3 a);
void entityTransformApply(Entity* entity);
void entityTransformReset(Entity* entity);
mat4 entityGetModelMatrix(Entity* entity);

Scene* sceneCreate();
void sceneDestroy(Scene* scene);
Entity* sceneCreateEntity(Scene* scene, Model* model);
void sceneDestoryEntity(Scene* scene, Entity* entity);
void sceneEntityApplyTransform(Scene* scene);
PointLight* sceneAddPointLight(Scene* scene);
DirectionalLight* sceneAddDirectionalLight(Scene* scene);
void sceneRemovePointLight(Scene* scene, PointLight* light);
void sceneRemoveDirectionalLight(Scene* scene, DirectionalLight* light);

void sceneEntityCreatePhysicsBody(Scene* scene, Entity* entity);

void calculate_model_AABB(Model* model);
void calculate_mesh_AABB(Mesh* mesh);
void load_default_images(HashMap* images);
Material* create_default_material(HashMap* images);

Model* assetLoadGLTF(const char* gltf_dir, const char* gltf_file);

Model* assetGenerateUVSphere(int slices, int stacks, float radius);