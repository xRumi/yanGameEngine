#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 normal;
layout (location = 4) in vec4 tangent;

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
} ubo;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragTexCoord;

layout (push_constant) uniform constant {
    mat4 model;
} PushConstant0;

void main() {
    gl_Position = ubo.projection * ubo.view * PushConstant0.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = texCoord;
}