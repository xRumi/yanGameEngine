#include "asset_manager.h"

Model* createEmptyModel(const char* name) {
    Model* model = memalloc(sizeof(Model), MEMORY_TAG_ASSET_MANAGER);
    model->name = name;
    model->materials = hashmap_create(1);
    model->images = hashmap_create(5);
    model->meshes = darray_create_reserve(Mesh, 1);
    model->nodes = hashmap_create(1);
    imagesPutDefaultImages(model->images);
    hashmap_put(model->materials, 0, (uint64_t)materialFromDefaultImages(model->images));
    return model;
}

Model* assetGenerateTriangle(vec3 a, vec3 b, vec3 c) {
    Model* model = createEmptyModel("Triangle");
    Material* material = (Material*)hashmap_get(model->materials, 0);
    Mesh mesh = {
        .vertices = darray_create_memoryTag(Vertex, MEMORY_TAG_ASSET_MANAGER),
        .indices = darray_create_memoryTag(uint32_t, MEMORY_TAG_ASSET_MANAGER),
        .material = material
    };

    Vertex vertex = {
        .position = {{0, 0, 0}},
        .color = {{1, 1, 1, 1}},
        .texCoord = {{0, 0}},
        .normal = {{0, 0, 1}},
        .tangent = {{1, 0, 0, 1}},
    };
    vertex.position = a;
    darray_push(mesh.vertices, vertex);
    vertex.position = b;
    darray_push(mesh.vertices, vertex);
    vertex.position = c;
    darray_push(mesh.vertices, vertex);
    darray_push(mesh.indices, 0);
    darray_push(mesh.indices, 1);
    darray_push(mesh.indices, 2);

    updateMeshColliderHalfDimensions(&mesh);
    mesh.collider.type = COLLIDER_TYPE_AABB;
    model->meshes[0] = mesh;

    model->collider = mesh.collider;
    updateModelColliderHalfDimensions(model);

    Node* node = memalloc(sizeof(Node), MEMORY_TAG_ASSET_MANAGER);
    *node = (Node){
        .mesh = &model->meshes[0],
        .matrix = mat4_identity()
    };
    hashmap_put(model->nodes, 0, (uint64_t)node);

    return model;
}

Model* assetGenerateRectangle(vec3 a, vec3 b, vec3 c, vec3 d, const char* baseColorPath) {
    Model* model = createEmptyModel("Rectangle");
    Material* material = (Material*)hashmap_get(model->materials, 0);
    if (baseColorPath) {
        Image* baseColorImage = imageLoadFromPath(baseColorPath);
        if (!baseColorImage) FATAL("Failed to load base color image");
        hashmap_put(model->images, hash_string(baseColorPath), (uint64_t)baseColorImage);
        material->baseColor.image = baseColorImage;
    }
    Mesh mesh = {
        .vertices = darray_create_memoryTag(Vertex, MEMORY_TAG_ASSET_MANAGER),
        .indices = darray_create_memoryTag(uint32_t, MEMORY_TAG_ASSET_MANAGER),
        .material = material
    };

    Vertex vertex = {
        .position = {{0, 0, 0}},
        .color = {{1, 1, 1, 1}},
        .texCoord = {{0, 0}},
        .normal = {{0, 0, 1}},
        .tangent = {{1, 0, 0, 1}},
    };
    vertex.position = a;
    vertex.texCoord = (vec2){{0, 0}};
    darray_push(mesh.vertices, vertex);

    vertex.position = b;
    vertex.texCoord = (vec2){{0, 1}};
    darray_push(mesh.vertices, vertex);

    vertex.position = c;
    vertex.texCoord = (vec2){{1, 1}};
    darray_push(mesh.vertices, vertex);

    vertex.position = d;
    vertex.texCoord = (vec2){{1, 0}};
    darray_push(mesh.vertices, vertex);

    darray_push(mesh.indices, 0);
    darray_push(mesh.indices, 1);
    darray_push(mesh.indices, 2);
    darray_push(mesh.indices, 2);
    darray_push(mesh.indices, 3);
    darray_push(mesh.indices, 0);

    updateMeshColliderHalfDimensions(&mesh);
    mesh.collider.type = COLLIDER_TYPE_AABB;
    model->meshes[0] = mesh;

    model->collider = mesh.collider;
    updateModelColliderHalfDimensions(model);

    Node* node = memalloc(sizeof(Node), MEMORY_TAG_ASSET_MANAGER);
    *node = (Node){
        .mesh = &model->meshes[0],
        .matrix = mat4_identity()
    };
    hashmap_put(model->nodes, 0, (uint64_t)node);

    return model;
}

Model* assetGenerateUVSphere(int slices, int stacks, float radius, vec4 color) {
    Model* model = createEmptyModel("Sphere");
    Material* material = (Material*)hashmap_get(model->materials, 0);
    Mesh mesh = {
        .vertices = darray_create_memoryTag(Vertex, MEMORY_TAG_ASSET_MANAGER),
        .indices = darray_create_memoryTag(uint32_t, MEMORY_TAG_ASSET_MANAGER),
        .material = material
    };
    for (int i = 0; i <= stacks; i++) {
        float v = (float)i / stacks;
        float phi = (v - 0.5f) * M_PI;

        for (int j = 0; j <= slices; j++) {
            float u = (float)j / slices;
            float theta = u * 2.0f * M_PI;

            float x = radius * cos(phi) * cos(theta);
            float y = radius * sin(phi);
            float z = radius * cos(phi) * sin(theta);

            Vertex vertex = {};
            vertex.position = (vec3){{x, y, z}};
            vertex.color = color;
            vertex.normal = vec3_normalize(vertex.position);
            vertex.texCoord = (vec2){{u, v}};
            darray_push(mesh.vertices, vertex);
        }
    }
    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {

            int i0 = i * (slices + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (slices + 1);
            int i3 = i2 + 1;

            // triangle 1
            darray_push(mesh.indices, i0);
            darray_push(mesh.indices, i2);
            darray_push(mesh.indices, i1);

            // triangle 2
            darray_push(mesh.indices, i1);
            darray_push(mesh.indices, i2);
            darray_push(mesh.indices, i3);
        }
    }
    updateMeshColliderHalfDimensions(&mesh);
    mesh.collider.radius = radius;
    mesh.collider.type = COLLIDER_TYPE_SPHERE;
    model->meshes[0] = mesh;

    model->collider = mesh.collider;
    updateModelColliderHalfDimensions(model);

    Node* node = memalloc(sizeof(Node), MEMORY_TAG_ASSET_MANAGER);
    *node = (Node){
        .mesh = &model->meshes[0],
        .matrix = mat4_identity()
    };
    hashmap_put(model->nodes, 0, (uint64_t)node);

    return model;
}
