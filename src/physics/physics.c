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

typedef struct CollisionResult {
    vec3 normal;
    float penetration;
} CollisionResult;

CollisionResult resolveCollisionSphereToSphere(BoundingSphere target, BoundingSphere other) {
    vec3 towardA = vec3_sub(target.center, other.center);
    float distance = vec3_length(towardA);
    float penetration = (target.radius + other.radius) - distance;
    if (penetration < 0 || distance == 0) return (CollisionResult){};
    penetration *= 0.5f;
    return (CollisionResult){
        .normal = vec3_normalize(towardA),
        .penetration = penetration
    };
}

CollisionResult resolveCollisionAabbToAabb(AABB target, AABB other) {
    if ((target.min.x <= other.max.x && target.max.x >= other.min.x) &&
        (target.min.y <= other.max.y && target.max.y >= other.min.y) &&
        (target.min.z <= other.max.z && target.max.z >= other.min.z)) {
        vec3 centerA = vec3_scale(vec3_add(target.min, target.max), 0.5);
        vec3 centerB = vec3_scale(vec3_add(other.min, other.max), 0.5);
        vec3 towardA = vec3_sub(centerA, centerB);
        vec3 overlap = {{
            MIN(target.max.x, other.max.x) - MAX(target.min.x, other.min.x),
            MIN(target.max.y, other.max.y) - MAX(target.min.y, other.min.y),
            MIN(target.max.z, other.max.z) - MAX(target.min.z, other.min.z),
        }};
        return (CollisionResult) {
            .normal = vec3_normalize(towardA),
            .penetration = overlap.y
        };
    }
    return (CollisionResult){};
}

CollisionResult resolveCollisionByTypes(Collider* target, Collider* other) {
    if (target->type == COLLIDER_TYPE_AABB && other->type == COLLIDER_TYPE_AABB) return resolveCollisionAabbToAabb(target->aabb, other->aabb);
    if (target->type == COLLIDER_TYPE_SPHERE && other->type == COLLIDER_TYPE_SPHERE) return resolveCollisionSphereToSphere(target->boundingSphere, other->boundingSphere);
    return (CollisionResult){};
}

vec3 resolveCollisionVelocity(vec3 velocity, vec3 normal) {
    float dot = vec3_dot(velocity, normal);
    if (dot < 0) return vec3_scale(normal, dot);
    return (vec3){};
}

void resolveCollisions(PhysicsBody* target, PhysicsBody** bodies, float dt, int subSteps) {
    for (int i = 0; i < subSteps; i++) {
        PhysicsBody* other;
        darray_foreach(bodies, other) {
            if (other == target) continue;
            CollisionResult collisionResult = resolveCollisionByTypes(target->collider, other->collider);
            target->velocity = vec3_sub(target->velocity, resolveCollisionVelocity(target->velocity, collisionResult.normal));
            target->transform->translation = vec3_add(target->transform->translation, vec3_scale(collisionResult.normal, collisionResult.penetration));
            if (!other->isStatic) {
                other->velocity = vec3_sub(other->velocity, resolveCollisionVelocity(other->velocity, vec3_neg(collisionResult.normal)));
                other->transform->translation = vec3_add(other->transform->translation, vec3_scale(vec3_neg(collisionResult.normal), collisionResult.penetration));
            }
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
        resolveCollisions(physicsBody, engine->bodies, dt, 1);
        // printf("(%.2f, %.2f, %.2f)\n", physicsBody->transform->translation.x, physicsBody->transform->translation.y, physicsBody->transform->translation.z);
    }
}
void physicsBodyMassSet(PhysicsBody* physicsBody, float mass) {
    physicsBody->mass = mass;
    physicsBody->inverseMass = 1.0 / mass;
}
void physicsBodyStaticSet(PhysicsBody* physicsBody, bool isStatic) {
    physicsBody->isStatic = isStatic;
}
