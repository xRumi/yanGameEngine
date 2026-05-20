#pragma once
#include <stdbool.h>
#include "math_types.h"

typedef struct Transform {
    vec3 translation;
    vec3 rotation;
    vec3 scale;
} Transform;

typedef enum ColliderType {
    COLLIDER_TYPE_AABB,
    COLLIDER_TYPE_SPHERE,
    COLLIDER_TYPE_NONE,
    COLLIDER_TYPE_MAX
} ColliderType;

typedef struct Collider {
    ColliderType type;
    vec3* center;
    float radius;
    vec3 halfDimensions;
} Collider;

typedef struct CollisionResult {
    vec3 normal;
    float penetration;
} CollisionResult;


typedef struct PhysicsBody {
    Transform* transform;
    vec3 velocity;
    vec3 acceleration;
    vec3 forceAccumulator;
    float gravity;
    float mass;
    float massInverse;

    Collider* collider;
    bool isCollidable;
    bool isStatic;
} PhysicsBody;

typedef struct PhysicsEngine {
    PhysicsBody** bodies;
    float gravity;
} PhysicsEngine;

PhysicsEngine* physicsEngineCreate();
void physicsEngineRun(PhysicsEngine* engine, float dt);
void physicsEngineGravitySet(PhysicsEngine* physicsEngine, float gravity);
void physicsBodyGravitySet(PhysicsBody* physicsBody, float gravity);
void physicsBodyMassSet(PhysicsBody* physicsBody, float mass);
void physicsBodyStaticSet(PhysicsBody* physicsBody, bool isStatic);

void physicsBodySetCollidable(PhysicsBody* physicsBody, bool isCollidable);
void physicsBodyAddForce(PhysicsBody* physicsBody, vec3 force);
void physicsBodyAddVelocity(PhysicsBody* physicsBody, vec3 velocity);

void resolveCollisions(PhysicsBody* target, PhysicsBody** bodies, float dt);
bool isCollisionSphereToAabb(Collider* target, Collider* other);