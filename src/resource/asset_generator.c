#include "asset_manager.h"

Model* assetGenerateUVSphere(int slices, int stacks, float radius) {
    Model* model = memalloc(sizeof(Model), MEMORY_TAG_ASSET_MANAGER);
    model->name = "Sphere";
    model->materials = hashmap_create(1);
    model->images = hashmap_create(5);

    load_default_images(model->images);
    hashmap_put(model->materials, 0, (uint64_t)create_default_material(model->images));
    Material* material = (Material*)hashmap_get(model->materials, 0);
    Mesh mesh = {
        .vertices = darray_create_memoryTag(Vertex, MEMORY_TAG_ASSET_MANAGER),
        .indices = darray_create_memoryTag(uint32_t, MEMORY_TAG_ASSET_MANAGER),
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
            vertex.color = (vec4){{1, 1, 1, 1}};
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
    calculate_mesh_AABB(&mesh);
    mesh.collider.radius = radius;
    mesh.collider.type = COLLIDER_TYPE_SPHERE;
    darray_push(material->meshes, mesh);

    model->collider = mesh.collider;
    calculate_model_AABB(model);

    return model;
}