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

typedef enum ColliderType {
    COLLIDER_TYPE_AABB,
} ColliderType;

typedef struct PhysicsBody {
    Transform* transform;
    vec3 velocity;
    vec3 acceleration;
    float mass, inverseMass;
    bool isStatic;

    vec3 forceAccumulator;

    AABB* aabb;
    ColliderType colliderType;
} PhysicsBody;

typedef struct PhysicsEngine {
    PhysicsBody** bodies;
    double gravity;
} PhysicsEngine;

PhysicsEngine* physicsEngineCreate();
void physicsEngineRun(PhysicsEngine* engine, float dt);
void physicsBodyMassSet(PhysicsBody* physicsBody, float mass);
void physicsBodyStaticSet(PhysicsBody* physicsBody, bool isStatic);