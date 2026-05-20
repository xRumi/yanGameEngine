#include "physics.h"
#include "utils.h"
#include "emath.h"

bool isCollisionSphereToSphere(Collider* target, Collider* other) {
    vec3 towardOther = vec3_sub(*other->center, *target->center);
    float totalRadius = target->radius + other->radius;
    float distance = vec3_length(towardOther);
    if (distance <= totalRadius) return true;
    return false;
}
bool resolveCollisionSphereToSphere(Collider* target, Collider* other, CollisionResult* collisionResult) {
    vec3 towardOther = vec3_sub(*other->center, *target->center);
    float totalRadius = target->radius + other->radius;
    float distance = vec3_length(towardOther);
    if (distance > totalRadius) return false;
    if (distance < 1e-7) {
        WARN("VERY CLOSE SPHERE TO SPHERE COLLIESION = %f", distance);
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

bool isCollisionSphereToAabb(Collider* target, Collider* other) {
    vec3 closestPointInAABBFromSphere = {
        .x = clamp(target->center->x, other->center->x - other->halfDimensions.x, other->center->x + other->halfDimensions.x),
        .y = clamp(target->center->y, other->center->y - other->halfDimensions.y, other->center->y + other->halfDimensions.y),
        .z = clamp(target->center->z, other->center->z - other->halfDimensions.z, other->center->z + other->halfDimensions.z)
    };
    vec3 towardOther = vec3_sub(closestPointInAABBFromSphere, *target->center);
    float distance = vec3_length(towardOther);
    if (distance <= target->radius) {
        return true;
    }
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
    PhysicsBody** otherRef;
    darray_foreach(bodies, otherRef) {
        PhysicsBody* other = *otherRef;
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