#pragma once
#include "asset_types.h"
#include "emath.h"
#include "defines.h"
#include "math_types.h"
#include "darray.h"

Model* assetLoadGLTF(const char* gltf_dir, const char* gltf_file);
Model* assetGenerateUVSphere(int slices, int stacks, float radius, vec4 color);
Model* assetGenerateTriangle(vec3 a, vec3 b, vec3 c);
Model* assetGenerateRectangle(vec3 a, vec3 b, vec3 c, vec3 d, const char* baseColorPath);

Image* imageLoadFromPath(const char* path);
void updateModelColliderHalfDimensions(Model* model);
void updateMeshColliderHalfDimensions(Mesh* mesh);
void imagesPutDefaultImages(HashMap* images);
Material* materialFromDefaultImages(HashMap* images);
mat4 mat4FromTransform(Transform transform);

Entity* entityCreate(Model* model, bool isHidden);
void entityDestroy(Entity* entity);
void entitySetHidden(Entity* entity, bool isHidden);
void entityTransformSetTranslation(Entity* entity, vec3 a);
void entityTransformSetTranslationX(Entity* entity, float x);
void entityTransformSetTranslationY(Entity* entity, float y);
void entityTransformSetTranslationZ(Entity* entity, float z);
void entityTransformSetScale(Entity* entity, vec3 scale);
void entityTransformApply(Entity* entity);
void entityTransformReset(Entity* entity);
mat4 entityGetModelMatrix(Entity* entity);
void entityPhysicsBodyAddForce(Entity* entity, vec3 force);
void entityPhysicsBodyAddVelocity(Entity* entity, vec3 velocity);
HashMap* entityCreateNodeAnimations(Model* model);
void entityNodeAnimationApply(Entity* entity);
void entityCreatePhysicsBody(Entity* entity);

Scene* sceneCreate();
void sceneDestroy(Scene* scene);
void sceneAddEntity(Scene* scene, Entity* entity);
Entity* sceneCreateEntity(Scene* scene, Model* model);
Entity* sceneCreateEntityHidden(Scene* scene, Model* model);
void sceneRemoveEntity(Scene* scene, Entity* entity);
void sceneEntityApplyTransform(Scene* scene);
Light* sceneCreatePointLight(Scene* scene, Model* lightModel, PointLight pointLight);
Entity* scenePointLightGetEntity(Scene* scene, Light* light);
void sceneDestroyLight(Scene* scene, Light* light);
Light* sceneCreateDirectionalLight(Scene* scene, DirectionalLight directionalLight);
void sceneCameraSetPosition(Scene* scene, vec3 position);
