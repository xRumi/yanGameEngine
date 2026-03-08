#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 texCoord;

layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = texCoord;
}