#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 fragPosition;
layout (location = 3) in vec3 fragNormal;

struct PointLight {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
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
    float pointLightCount, directionalLightCount;
    float f_reserve0, f_reserve1;
    PointLight pointLights[POINT_LIGHT_MAX_COUNT];
    DirectionalLight directionalLights[DIRECTIONAL_LIGHT_MAX_COUNT];
} lightUBO;

layout (set = 1, binding = 0) uniform sampler2D baseColorSampler;
layout (set = 1, binding = 1) uniform sampler2D normalSampler;
layout (set = 1, binding = 2) uniform sampler2D metallicRoughnessSampler;

layout (location = 0) out vec4 outColor;

vec4 CalcDirectionalLight(DirectionalLight light, vec4 baseColor, vec4 normal, vec4 cameraDirection) {
    vec4 lightDirection = normalize(-light.direction);
    vec4 lightReflection = reflect(-lightDirection, normal);
    float diffuseFactor = max(dot(lightDirection, normal), 0.0);
    float specularFactor = pow(max(dot(cameraDirection, lightReflection), 0.0), 64.0);
    vec4 ambient  = light.ambient * baseColor;
    vec4 diffuse  = light.diffuse * diffuseFactor * baseColor;
    vec4 specular = light.specular * specularFactor * 0.5;
    return (ambient + diffuse + specular);
}
vec4 CalcPointLight(PointLight light, vec4 baseColor, vec4 normal, vec4 cameraDirection) {
    vec4 lightDirection = normalize(light.position - vec4(fragPosition, 0));
    vec4 lightReflection = reflect(-lightDirection, normal);
    float diffuseFactor = max(dot(lightDirection, normal), 0.0);
    float specularFactor = pow(max(dot(cameraDirection, lightReflection), 0.0), 64.0);
    float distance = length(light.position - vec4(fragPosition, 0));
    float attenuation = 1.0 / (1 + light.linear * distance + light.quadratic * (distance * distance));
    vec4 ambient  = light.ambient * baseColor;
    vec4 diffuse  = light.diffuse * diffuseFactor * baseColor;
    vec4 specular = light.specular * specularFactor * 0.5;
    return (ambient + diffuse + specular) * attenuation;
}

void main() {
    vec4 baseColor = texture(baseColorSampler, fragTexCoord);
    vec4 cameraDirection = normalize(frameUBO.cameraPosition - vec4(fragPosition, 0));
    vec4 normal = normalize(vec4(fragNormal, 0));

    outColor = vec4(0);
    for (int i = 0; i < lightUBO.directionalLightCount; i++)
        outColor += CalcDirectionalLight(lightUBO.directionalLights[i], baseColor, normal, cameraDirection);
    for (int i = 0; i < lightUBO.pointLightCount; i++)
        outColor += CalcPointLight(lightUBO.pointLights[i], baseColor, normal, cameraDirection);
    outColor *= fragColor;
}