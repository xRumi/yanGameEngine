#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 fragPosition;
layout (location = 3) in vec3 fragNormal;

struct PointLight {
    vec4 position;
    vec4 ambient;
    vec4 difusse;
    vec4 specular;
    float linear;
    float quadratic;
};
struct DirectionalLight {
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
#define POINT_LIGHT_MAX_COUNT 32
#define DIRECTIONAL_LIGHT_MAX_COUNT 16
layout (set = 0, binding = 0) uniform UniformBufferObject0 {
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
} frameUBO;
layout (set = 0, binding = 1) uniform UniformBufferObject1 {
    PointLight pointLights[POINT_LIGHT_MAX_COUNT];
    DirectionalLight directionalLights[DIRECTIONAL_LIGHT_MAX_COUNT];
} lightUBO;

layout (set = 1, binding = 0) uniform sampler2D baseColorSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D metallicRoughnessSampler;

layout (location = 0) out vec4 outColor;

vec3 CalcDirectionalLight(DirectionalLight light, vec3 baseColor, vec3 normal, vec3 cameraDirection) {
    vec3 fragToLight = vec3(normalize(-light.direction));
    vec3 lightReflection = reflect(-fragToLight, normal);
    float diffuseFactor = max(dot(fragToLight, normal), 0.0);
    float specularFactor = pow(max(dot(cameraDirection, lightReflection), 0.0), 64.0);
    vec3 ambient  = vec3(light.ambient)  * baseColor;
    vec3 diffuse  = vec3(light.diffuse)  * diffuseFactor * baseColor;
    vec3 specular = vec3(light.specular) * specularFactor * 0.5;
    return (ambient + diffuse + specular);
}

void main() {
    vec3 baseColor = vec3(texture(baseColorSampler, fragTexCoord));
    vec3 cameraDirection = normalize(vec3(frameUBO.cameraPosition) - fragPosition);
    vec3 normal = normalize(fragNormal);

    outColor = vec4(0);
    for (int i = 0; i < 1; i++) {
        vec3 directionalLight = CalcDirectionalLight(lightUBO.directionalLights[i], baseColor, normal, cameraDirection);
        outColor += vec4(directionalLight, 0);
    }
    outColor *= fragColor;
}