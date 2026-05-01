#include "asset_manager.h"

void updateNodeAnimationGeneration(Entity* entity, NodeAnimation* nodeAnimation) {
    if (nodeAnimation->generation < entity->generation) {
        nodeAnimation->propagration = mat4_identity();
        nodeAnimation->generation = entity->generation;
    }
}
void propagateNodeTransform(Entity* entity, NodeAnimation* nodeAnimation) {
    mat4 parentPropagraion = nodeAnimation->propagration;
    darray_foreach_inline_decl(nodeAnimation->node->child, Node*, n) {
        NodeAnimation* childAnimation = (NodeAnimation*)hashmap_get(entity->nodeAnimations, (uint64_t)n);
        updateNodeAnimationGeneration(entity, childAnimation);
        childAnimation->propagration = mat4_mul(parentPropagraion, childAnimation->propagration);
        propagateNodeTransform(entity, childAnimation);
    }
}
void entityNodeAnimationApply(Entity* entity) {
    NodeAnimation* nodeAnimation;
    hashmap_foreach(entity->nodeAnimations, nodeAnimation) {
        updateNodeAnimationGeneration(entity, nodeAnimation);
        Node* node = nodeAnimation->node;
        Transform transform = {
            .scale = {{1, 1, 1}}
        };
        if (node->animationSampler.translation.input) {
            float* inputs = node->animationSampler.translation.input;
            double animationTime = fmod(entity->timeManager.elapsedTime, node->animationSampler.translation.inputMax);
            int i = 1;
            for (; i < darray_get_length(inputs); i++)
                if (animationTime <= inputs[i]) break;
            vec3 start = node->animationSampler.translation.output[i - 1], end = node->animationSampler.translation.output[i];
            transform.translation = vec3_lerp(start, end, (animationTime - node->animationSampler.translation.input[i - 1]) / (node->animationSampler.translation.input[i] - node->animationSampler.translation.input[i - 1]));
        }
        nodeAnimation->propagration = mat4_mul(nodeAnimation->propagration, mat4FromTransform(transform));
        propagateNodeTransform(entity, nodeAnimation);
    }
    hashmap_foreach(entity->nodeAnimations, nodeAnimation) {
        atomicMatrixSetMatrix(&nodeAnimation->matrix, nodeAnimation->propagration);
    }
}