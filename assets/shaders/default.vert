#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec4 inTangent;
layout (location = 5) in vec4 inWeights;
layout (location = 6) in vec4 inJoints;

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
} ubo;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragPosition;
layout (location = 3) out vec3 fragNormal;

layout (push_constant) uniform constant {
    mat4 model;
    mat4 node;
} PushConstant0;

layout (std430, set = 2, binding = 0) readonly buffer SSBO1 {
    mat4 node;
    mat4 inverseBind[1024];
    mat4 joints[1024];
} SSBO_1;

void main() {
    vec4 rest = vec4(inPosition, 1.0);

    mat4 boneSpace = 
        SSBO_1.inverseBind[int(inJoints.x)] +
        SSBO_1.inverseBind[int(inJoints.y)] +
        SSBO_1.inverseBind[int(inJoints.z)] +
        SSBO_1.inverseBind[int(inJoints.w)];

    mat4 skinned = 
        inWeights.x * SSBO_1.joints[int(inJoints.x)] +
        inWeights.y * SSBO_1.joints[int(inJoints.y)] +
        inWeights.z * SSBO_1.joints[int(inJoints.z)] +
        inWeights.w * SSBO_1.joints[int(inJoints.w)];

    mat4 model = PushConstant0.model;
    gl_Position = ubo.projection * ubo.view * model * skinned * boneSpace * rest;
    fragPosition = vec3(PushConstant0.model * vec4(inPosition, 1.0));
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    mat4 normalMatrix = transpose(inverse(model));
    fragNormal = vec3(normalMatrix * vec4(inNormal, 1.0));
}