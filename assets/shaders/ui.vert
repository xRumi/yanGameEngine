#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragUV;
layout (location = 2) flat out uint fragChar;


layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
} ubo;

#define MAX_GLYPHS 65536
struct UICharacterInstance {
    vec3 position;
    uint color;
    vec2 size;
    uint character;
    uint reserve[1];
};
layout (std430, set = 1, binding = 0) readonly buffer SSBO_0 {
    UICharacterInstance UICharacterInstances[MAX_GLYPHS];
} UISSBO_0;

layout (push_constant) uniform constant {
    int uiComponentType;
} PushConstant;

void main() {
    if (PushConstant.uiComponentType == 0) {
        // UIComponentType = UI_TEXT

        UICharacterInstance uICharacterInstance = UISSBO_0.UICharacterInstances[gl_InstanceIndex];

        vec2 corners[4] = vec2[](
            vec2(uICharacterInstance.position.x, uICharacterInstance.position.y),
            vec2(uICharacterInstance.position.x, uICharacterInstance.position.y + uICharacterInstance.size.y),
            vec2(uICharacterInstance.position.x + uICharacterInstance.size.x, uICharacterInstance.position.y),
            vec2(uICharacterInstance.position.x + uICharacterInstance.size.x, uICharacterInstance.position.y + uICharacterInstance.size.y)
        );
        vec2 uvs[4] = vec2[](
            vec2(0, 0), vec2(0, 1),
            vec2(1, 0), vec2(1, 1)
        );

        gl_Position = vec4(corners[gl_VertexIndex], 0, 1);
        fragColor = vec4(
            ((uICharacterInstance.color >> 24) & 0xFF) / 256.0,
            ((uICharacterInstance.color >> 16) & 0xFF) / 256.0,
            ((uICharacterInstance.color >> 8) & 0xFF) / 256.0,
            ((uICharacterInstance.color >> 0) & 0xFF) / 256.0
        );
        fragUV = uvs[gl_VertexIndex];
        fragChar = uICharacterInstance.character;
    } else {
        // do nothing
        gl_Position = vec4(0);
    }
}