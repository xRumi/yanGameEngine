#pragma once
#include "defines.h"
#include "emath.h"

typedef struct Transform {
    vec3 translation;
    vec3 rotation;
    vec3 scale;
} Transform;

typedef struct AABB {
    vec3 min;
    vec3 max;
} AABB;

typedef struct BoundingSphere {
    vec3 center;
    float radius;
} BoundingSphere;

typedef enum ColliderType {
    COLLIDER_TYPE_AABB,
    COLLIDER_TYPE_SPHERE,
    COLLIDER_TYPE_NONE,
    COLLIDER_TYPE_MAX
} ColliderType;

typedef struct Collider {
    ColliderType type;
    AABB aabb;
    BoundingSphere boundingSphere;
} Collider;

typedef struct PhysicsBody {
    Transform* transform;
    vec3 velocity;
    vec3 acceleration;
    float mass, inverseMass;
    bool isStatic;

    vec3 forceAccumulator;

    Collider* collider;
    bool isCollidable;
} PhysicsBody;

typedef struct PhysicsEngine {
    PhysicsBody** bodies;
    double gravity;
} PhysicsEngine;

PhysicsEngine* physicsEngineCreate();
void physicsEngineRun(PhysicsEngine* engine, float dt);
void physicsBodyMassSet(PhysicsBody* physicsBody, float mass);
void physicsBodyStaticSet(PhysicsBody* physicsBody, bool isStatic);

void physicsBodySetCollidable(PhysicsBody* physicsBody, bool isCollidable);
void physicsBodyAddForce(PhysicsBody* physicsBody, vec3 force);