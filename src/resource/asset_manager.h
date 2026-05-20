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

Entity* entityCreate(Model* model);
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
void sceneDestoryEntity(Scene* scene, Entity* entity);
void sceneEntityApplyTransform(Scene* scene);
PointLight* sceneAddPointLight(Scene* scene);
DirectionalLight* sceneAddDirectionalLight(Scene* scene);
void sceneRemovePointLight(Scene* scene, PointLight* light);
void sceneRemoveDirectionalLight(Scene* scene, DirectionalLight* light);
void sceneCameraSetPosition(Scene* scene, vec3 position);
