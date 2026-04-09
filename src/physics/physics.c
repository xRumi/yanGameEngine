#include "physics.h"
#include "darray.h"

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
    physicsBody->forceAccumulator = vec3_add(physicsBody->forceAccumulator, (vec3){{0, -gravity, 0}});
}

PhysicsEngine* physicsEngineCreate() {
    PhysicsEngine* physicsEngine = memalloc(sizeof(PhysicsEngine), MEMORY_TAG_PHYSICS);
    physicsEngine->bodies = darray_create_memoryTag(PhysicsBody, MEMORY_TAG_PHYSICS);
    physicsEngine->gravity = 9.8;
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
    }
}
void physicsBodyMassSet(PhysicsBody* physicsBody, float mass) {
    physicsBody->mass = mass;
    physicsBody->inverseMass = 1.0 / mass;
}
void physicsBodyStaticSet(PhysicsBody* physicsBody, bool isStatic) {
    physicsBody->isStatic = isStatic;
}
