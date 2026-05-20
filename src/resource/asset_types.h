#pragma once

#include "defines.h"
#include "math_types.h"
#include "hashMap.h"
#include "physics.h"
#include "utils.h"

typedef struct Vertex {
    vec3 position;
    vec4 color;
    vec2 texCoord;
    vec3 normal;
    vec4 tangent;
} Vertex;

// typedef struct Vertex {
//     uint16_t joints[4];
//     float weights[4];
// } Vertex;

// default values
// joints -> (0,0,0,0)
// weights -> (1,0,0,0)

typedef enum PipelineType {
    PIPELINE_TYPE_DEFAULT,
    PIPELINE_TYPE_WIREFRAME,
    PIPELINE_TYPE_MAX
} PipelineType;

typedef struct Image {
    void* data;
    uint32_t width, height;
    void* imageRendererStateRef;
} Image;

typedef struct Texture {
    Image* image;
} Texture;

typedef struct MeshAnimation {
    float* input;
    Transform* output;
} MeshAnimation;

typedef struct Material {
    Texture baseColor;
    Texture normal;
    Texture metallicRoughness;
    Texture occlusion;
    Texture emissive;
    float metallicFactor;
    float roughnessFactor;

    PipelineType pipelineType;
    void* materialRendererStateRef;
} Material;

typedef struct Mesh {
    Vertex* vertices;
    uint32_t* indices;
    Material* material;
    Collider collider;
    void* meshRendererStateRef;
} Mesh;

typedef struct NodeAnimationSampler {
    struct {
        float* input;
        float inputMax;
        vec4* output;
    } rotation;
    struct {
        float* input;
        float inputMax;
        vec3* output;
    } translation;
    struct {
        float* input;
        float inputMax;
        vec3* output;
    } scale;
} NodeAnimationSampler;

typedef struct Node {
    mat4 matrix;
    Mesh* mesh;
    struct Node** child;
    bool isAnimated;
    NodeAnimationSampler animationSampler;
} Node;

typedef struct NodeAnimation {
    Node* node;
    mat4 propagration;
    int generation;
    AtomicMatrix matrix;
} NodeAnimation;

typedef struct Model {
    const char* name;
    Mesh* meshes;
    HashMap* images;
    HashMap* materials;
    HashMap* nodes;
    Collider collider;
    bool isRendererReady;
} Model;

#define POINT_LIGHT_MAX_COUNT 32
#define DIRECTIONAL_LIGHT_MAX_COUNT 16
typedef struct __attribute__((aligned(16))) PointLight {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float linear;
    float quadratic;
} PointLight;
typedef struct __attribute__((aligned(16))) DirectionalLight {
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} DirectionalLight;

typedef struct Camera {
    AtomicVec3 position;
    AtomicVec3 rotation;
    float sensitivity;
} Camera;
typedef struct __attribute__((aligned(16))) FrameUBO {
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
} FrameUBO;
typedef struct __attribute__((aligned(16))) LightUBO {
    float pointLightCount, directionalLightCount;
    float f_reserve[2];
    PointLight pointLights[POINT_LIGHT_MAX_COUNT];
    DirectionalLight directionalLights[DIRECTIONAL_LIGHT_MAX_COUNT];
} LightUBO;

typedef struct Scene {
    Camera camera;
    FrameUBO frameUBO;
    LightUBO lightUBO;
    HashMap* entities; // TODO: make thread safe
    PhysicsEngine* physicsEngine;
} Scene;

typedef struct Entity {
    uint64_t id;
    Scene* scene;
    Model* model;
    HashMap* nodeAnimations;
    TimeManager timeManager;
    Transform transform;
    AtomicMatrix modelMatrix;
    PhysicsBody* physicsBody;
    Collider collider;
    bool isHidden;
    int generation;
} Entity;