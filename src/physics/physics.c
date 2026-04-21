#include "physics.h"
#include "darray.h"
#include "utils.h"

void applyForceAccumulator(PhysicsBody* physicsBody) {
    physicsBody->acceleration = vec3_scale(physicsBody->forceAccumulator, physicsBody->inverseMass);
    physicsBody->forceAccumulator = (vec3){};
}
void applyAcceleration(PhysicsBody* physicsBody, float dt) {
    physicsBody->velocity = vec3_add(physicsBody->velocity, vec3_scale(physicsBody->acceleration, dt));
}
void applyVelocity(PhysicsBody* physicsBody, float dt) {
    physicsBody->transform->translation = vec3_add(physicsBody->transform->translation, vec3_scale(physicsBody->velocity, dt));
}
void applyGravity(PhysicsBody* physicsBody, float gravity) {
    physicsBodyAddForce(physicsBody, (vec3){{0, -gravity * physicsBody->mass, 0}});
}

typedef struct CollisionResult {
    vec3 normal;
    float penetration;
} CollisionResult;

bool resolveCollisionSphereToSphere(Collider* target, Collider* other, CollisionResult* collisionResult) {
    vec3 towardOther = vec3_sub(*other->center, *target->center);
    float totalRadius = target->radius + other->radius;
    float distance = vec3_length(towardOther);
    if (distance > totalRadius) return false;
    if (distance < 1e-7) {
        WARN("DISTANCE = %f", distance);
        *collisionResult = (CollisionResult){
            .normal = (vec3){{0, 1, 0}},
            .penetration = totalRadius
        };
        return true;
    }
    float penetration = totalRadius - distance;
    *collisionResult = (CollisionResult){
        .normal = vec3_normalize(towardOther),
        .penetration = penetration
    };
    return true;
}

bool resolveCollisionAabbToAabb(Collider* target, Collider* other, CollisionResult* collisionResult) {
    return false;
}

bool resolveCollisionByTypes(Collider* target, Collider* other, CollisionResult* collisionResult) {
    if (target->type == COLLIDER_TYPE_AABB && other->type == COLLIDER_TYPE_AABB) return resolveCollisionAabbToAabb(target, other, collisionResult);
    if (target->type == COLLIDER_TYPE_SPHERE && other->type == COLLIDER_TYPE_SPHERE) return resolveCollisionSphereToSphere(target, other, collisionResult);
    return false;
}

vec3 resolveCollisionVelocity(vec3 velocity, vec3 normal) {
    float dot = vec3_dot(velocity, normal);
    if (dot > 0) return vec3_sub(velocity, vec3_scale(normal, dot));
    return velocity;
}
vec3 resolveCollisionTranslation(vec3 translation, vec3 normal, float penetration) {
    return vec3_sub(translation, vec3_scale(normal, penetration));
}

void resolveCollisions(PhysicsBody* target, PhysicsBody** bodies, float dt) {
    PhysicsBody* other;
    darray_foreach(bodies, other) {
        if (!other->isCollidable || other == target) continue;
        CollisionResult collisionResult = {};
        if (!resolveCollisionByTypes(target->collider, other->collider, &collisionResult)) continue;
        target->velocity = resolveCollisionVelocity(target->velocity, collisionResult.normal);
        target->transform->translation = resolveCollisionTranslation(target->transform->translation, collisionResult.normal, collisionResult.penetration);
        if (!other->isStatic) {
            other->velocity = resolveCollisionVelocity(other->velocity, vec3_neg(collisionResult.normal));
            other->transform->translation = resolveCollisionTranslation(other->transform->translation, vec3_neg(collisionResult.normal), collisionResult.penetration);
        }
    }
}

PhysicsEngine* physicsEngineCreate() {
    PhysicsEngine* physicsEngine = memalloc(sizeof(PhysicsEngine), MEMORY_TAG_PHYSICS);
    physicsEngine->bodies = darray_create_memoryTag(PhysicsBody, MEMORY_TAG_PHYSICS);
    physicsEngine->gravity = 9.81;
    return physicsEngine;
}
void physicsEngineRun(PhysicsEngine* engine, float dt) {
    PhysicsBody* physicsBody;
    darray_foreach(engine->bodies, physicsBody) {
        if (physicsBody->isStatic) continue;
        applyGravity(physicsBody, engine->gravity);
        applyForceAccumulator(physicsBody);
        applyAcceleration(physicsBody, dt);
        applyVelocity(physicsBody, dt);
        if (physicsBody->isCollidable) resolveCollisions(physicsBody, engine->bodies, dt);
    }
}
void physicsBodyMassSet(PhysicsBody* physicsBody, float mass) {
    physicsBody->mass = mass;
    physicsBody->inverseMass = 1.0 / mass;
}
void physicsBodyStaticSet(PhysicsBody* physicsBody, bool isStatic) {
    physicsBody->isStatic = isStatic;
}
void physicsBodySetCollidable(PhysicsBody* physicsBody, bool isCollidable) {
    physicsBody->isCollidable = isCollidable;
}
void physicsBodyAddForce(PhysicsBody* physicsBody, vec3 force) {
    physicsBody->forceAccumulator = vec3_add(physicsBody->forceAccumulator, force);
}