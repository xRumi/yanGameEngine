#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec2 texCoord;

layout (set = 1, binding = 0) uniform sampler2D baseColorSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D metallicRoughnessSampler;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture(baseColorSampler, texCoord) * fragColor;
}