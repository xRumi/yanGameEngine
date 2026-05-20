#include "physics.h"
#include "darray.h"
#include "emath.h"

void applyForceAccumulator(PhysicsBody* physicsBody) {
    physicsBody->acceleration = vec3_scale(physicsBody->forceAccumulator, physicsBody->massInverse);
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

PhysicsEngine* physicsEngineCreate() {
    PhysicsEngine* physicsEngine = memalloc(sizeof(PhysicsEngine), MEMORY_TAG_PHYSICS);
    physicsEngine->bodies = darray_create_memoryTag(PhysicsBody*, MEMORY_TAG_PHYSICS);
    physicsEngine->gravity = 9.81;
    return physicsEngine;
}
void physicsEngineRun(PhysicsEngine* engine, float dt) {
    PhysicsBody** physicsBodyRef;
    darray_foreach(engine->bodies, physicsBodyRef) {
        PhysicsBody* physicsBody = *physicsBodyRef;
        if (physicsBody->isStatic) continue;
        applyGravity(physicsBody, physicsBody->gravity);
        applyForceAccumulator(physicsBody);
        applyAcceleration(physicsBody, dt);
        applyVelocity(physicsBody, dt);
        if (physicsBody->isCollidable) resolveCollisions(physicsBody, engine->bodies, dt);
    }
}
void physicsEngineGravitySet(PhysicsEngine* physicsEngine, float gravity) {
    physicsEngine->gravity = gravity;
}
void physicsBodyGravitySet(PhysicsBody* physicsBody, float gravity) {
    physicsBody->gravity = gravity;
}
void physicsBodyMassSet(PhysicsBody* physicsBody, float mass) {
    physicsBody->mass = mass;
    physicsBody->massInverse = 1.0 / mass;
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
void physicsBodyAddVelocity(PhysicsBody* physicsBody, vec3 velocity) {
    physicsBody->velocity = vec3_add(physicsBody->velocity, velocity);
}