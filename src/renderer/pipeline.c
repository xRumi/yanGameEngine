#include "renderer.h"

VkShaderModule createShaderModule(VkDevice device, const char* shaderCode) {
    VkShaderModule shaderModule = {};
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = darray_get_length(shaderCode);
    createInfo.pCode = (const uint32_t*)shaderCode;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        FATAL("Failed to create shader module");
    }
    return shaderModule;
}

void createGraphicsPipline(VkDevice device, PipelineOptions options, VkPipeline* pipeline, VkPipelineLayout* pipelineLayout) {
    char* vertShaderCode = readFile(options.vertShaderPath);
    char* fragShaderCode = readFile(options.fragShaderPath);
    VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);
    darray_destroy(vertShaderCode);
    darray_destroy(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicCreateInfo = {};
    dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicCreateInfo.dynamicStateCount = CARRAY_SIZE(dynamicStates);
    dynamicCreateInfo.pDynamicStates = dynamicStates;

    VkPipelineVertexInputStateCreateInfo vertexCreateInfo = {};
    vertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexCreateInfo.vertexBindingDescriptionCount = darray_get_length(options.vertexBindingDescriptions);
    vertexCreateInfo.pVertexBindingDescriptions = options.vertexBindingDescriptions;
    vertexCreateInfo.vertexAttributeDescriptionCount = darray_get_length(options.vertexAttributeDescriptions);
    vertexCreateInfo.pVertexAttributeDescriptions = options.vertexAttributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
    viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportCreateInfo.pViewports = &options.viewport;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pScissors = &options.scissor;
    viewportCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.polygonMode = options.polygonMode;
    rasterizationCreateInfo.lineWidth = 1.0f;
    rasterizationCreateInfo.cullMode = options.cullMode;
    rasterizationCreateInfo.frontFace = options.frontFace;
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = options.rasterizationSamples;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = options.blendEnable;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachment;

    VkPushConstantRange pushConstantRanges[1] = {};
    pushConstantRanges[0].size = sizeof(PushConstant0);
    pushConstantRanges[0].offset = 0;
    pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = darray_get_length(options.descriptorSetLayouts);
    pipelineLayoutCreateInfo.pSetLayouts = options.descriptorSetLayouts;
    pipelineLayoutCreateInfo.pushConstantRangeCount = CARRAY_SIZE(pushConstantRanges);
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges;

    if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, pipelineLayout) != VK_SUCCESS) {
        FATAL("Failed to create pipeline layout!");
    }
    
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = options.depthTestEnable;
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilCreateInfo.maxDepthBounds = 1.0f;
    depthStencilCreateInfo.minDepthBounds = 0.0f;

    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = 2;
    createInfo.pStages = shaderStageCreateInfos;
    createInfo.layout = *pipelineLayout;
    createInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    createInfo.pVertexInputState = &vertexCreateInfo;
    createInfo.pRasterizationState = &rasterizationCreateInfo;
    createInfo.pColorBlendState = &colorBlendCreateInfo;
    createInfo.pDynamicState = &dynamicCreateInfo;
    createInfo.pMultisampleState = &multisampleCreateInfo;
    createInfo.pViewportState = &viewportCreateInfo;
    createInfo.renderPass = options.renderPass;
    createInfo.pDepthStencilState = &depthStencilCreateInfo;
    createInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device, NULL, 1, &createInfo, NULL, pipeline) != VK_SUCCESS) {
        FATAL("Failed to create graphics pipeline");
    }
    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);
}

void createCommonPipelines(RendererState internalStateRenderer, PipelineState** pipelineStates) {
    *pipelineStates = darray_create_reserve(PipelineState, PIPELINE_TYPE_MAX);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)platformGetPlatformState()->width;
    viewport.height = (float)platformGetPlatformState()->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = (VkExtent2D){.width = platformGetPlatformState()->width, .height = platformGetPlatformState()->height};

    for (int i = 0; i < PIPELINE_TYPE_MAX; i++)
        switch (i) {
            case PIPELINE_TYPE_DEFAULT: {
                // create default pipeline
                PipelineState* pipeline = &(*pipelineStates)[i];

                VkVertexInputBindingDescription* vertexInputBindings = darray_create_reserve(VkVertexInputBindingDescription, 1);
                vertexInputBindings[0].binding = 0;
                vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                vertexInputBindings[0].stride = sizeof(Vertex);

                VkVertexInputAttributeDescription* vertexInputAttributeDescriptions = darray_create_reserve(VkVertexInputAttributeDescription, 5);
                vertexInputAttributeDescriptions[0].binding = 0;
                vertexInputAttributeDescriptions[0].location = 0;
                vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, position);

                vertexInputAttributeDescriptions[1].binding = 0;
                vertexInputAttributeDescriptions[1].location = 1;
                vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

                vertexInputAttributeDescriptions[2].binding = 0;
                vertexInputAttributeDescriptions[2].location = 2;
                vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                vertexInputAttributeDescriptions[2].offset = offsetof(Vertex, texCoord);

                vertexInputAttributeDescriptions[3].binding = 0;
                vertexInputAttributeDescriptions[3].location = 3;
                vertexInputAttributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
                vertexInputAttributeDescriptions[3].offset = offsetof(Vertex, normal);

                vertexInputAttributeDescriptions[4].binding = 0;
                vertexInputAttributeDescriptions[4].location = 4;
                vertexInputAttributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                vertexInputAttributeDescriptions[4].offset = offsetof(Vertex, tangent);

                VkDescriptorSetLayoutBinding* set_0_layoutBindings = darray_create_reserve(VkDescriptorSetLayoutBinding, 1);
                set_0_layoutBindings[0].binding = 0;
                set_0_layoutBindings[0].descriptorCount = 1;
                set_0_layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                set_0_layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                VkDescriptorSetLayoutCreateInfo set_0_layoutCreateInfo = {};
                set_0_layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                set_0_layoutCreateInfo.bindingCount = darray_get_length(set_0_layoutBindings);
                set_0_layoutCreateInfo.pBindings = set_0_layoutBindings;
                VkDescriptorSetLayout set_0_layout;
                if (vkCreateDescriptorSetLayout(internalStateRenderer.device, &set_0_layoutCreateInfo, NULL, &set_0_layout) != VK_SUCCESS) {
                    FATAL("Failed to create descriptor set layout");
                }

                VkDescriptorSetLayoutBinding* set_1_layoutBindings = darray_create_reserve(VkDescriptorSetLayoutBinding, 1);
                set_1_layoutBindings[0].binding = 0;
                set_1_layoutBindings[0].descriptorCount = 1;
                set_1_layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                set_1_layoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                VkDescriptorSetLayoutCreateInfo set_1_layoutCreateInfo = {};
                set_1_layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                set_1_layoutCreateInfo.bindingCount = darray_get_length(set_1_layoutBindings);
                set_1_layoutCreateInfo.pBindings = set_1_layoutBindings;
                VkDescriptorSetLayout set_1_layout;
                if (vkCreateDescriptorSetLayout(internalStateRenderer.device, &set_1_layoutCreateInfo, NULL, &set_1_layout) != VK_SUCCESS) {
                    FATAL("Failed to create descriptor set layout");
                }

                pipeline->descriptorSetLayouts = darray_create(VkDescriptorSetLayout);
                darray_push(pipeline->descriptorSetLayouts, set_0_layout);
                darray_push(pipeline->descriptorSetLayouts, set_1_layout);

                PipelineOptions options = {
                    .vertShaderPath = "assets/shaders/spv/default.vert.spv",
                    .fragShaderPath = "assets/shaders/spv/default.frag.spv",
                    .vertexBindingDescriptions = vertexInputBindings,
                    .vertexAttributeDescriptions = vertexInputAttributeDescriptions,
                    .descriptorSetLayouts = pipeline->descriptorSetLayouts,
                    .viewport = viewport,
                    .scissor = scissor,
                    .cullMode = VK_CULL_MODE_BACK_BIT,
                    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    .polygonMode = VK_POLYGON_MODE_FILL,
                    .depthTestEnable = VK_TRUE,
                    .blendEnable = VK_FALSE,
                    .rasterizationSamples = internalStateRenderer.msaaSamples,
                    .renderPass = internalStateRenderer.renderPass,
                };
                createGraphicsPipline(internalStateRenderer.device, options, &pipeline->pipeline, &pipeline->pipelineLayout);
                createPipelineFrameUBO(internalStateRenderer, pipeline);
                TRACE("Default graphics pipeline created");

                darray_destroy(vertexInputBindings);
                darray_destroy(vertexInputAttributeDescriptions);
                darray_destroy(set_0_layoutBindings);
                darray_destroy(set_1_layoutBindings);
                break;
            }
            case PIPELINE_TYPE_WIREFRAME: {
                // create wireframe pipeline
                PipelineState* pipeline = &(*pipelineStates)[i];

                VkVertexInputBindingDescription* vertexInputBindings = darray_create_reserve(VkVertexInputBindingDescription, 1);
                vertexInputBindings[0].binding = 0;
                vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                vertexInputBindings[0].stride = sizeof(Vertex);

                VkVertexInputAttributeDescription* vertexInputAttributeDescriptions = darray_create_reserve(VkVertexInputAttributeDescription, 5);
                vertexInputAttributeDescriptions[0].binding = 0;
                vertexInputAttributeDescriptions[0].location = 0;
                vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, position);

                vertexInputAttributeDescriptions[1].binding = 0;
                vertexInputAttributeDescriptions[1].location = 1;
                vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

                vertexInputAttributeDescriptions[2].binding = 0;
                vertexInputAttributeDescriptions[2].location = 2;
                vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                vertexInputAttributeDescriptions[2].offset = offsetof(Vertex, texCoord);

                vertexInputAttributeDescriptions[3].binding = 0;
                vertexInputAttributeDescriptions[3].location = 3;
                vertexInputAttributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
                vertexInputAttributeDescriptions[3].offset = offsetof(Vertex, normal);

                vertexInputAttributeDescriptions[4].binding = 0;
                vertexInputAttributeDescriptions[4].location = 4;
                vertexInputAttributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                vertexInputAttributeDescriptions[4].offset = offsetof(Vertex, tangent);

                VkDescriptorSetLayoutBinding* set_0_layoutBindings = darray_create_reserve(VkDescriptorSetLayoutBinding, 1);
                set_0_layoutBindings[0].binding = 0;
                set_0_layoutBindings[0].descriptorCount = 1;
                set_0_layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                set_0_layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                VkDescriptorSetLayoutCreateInfo set_0_layoutCreateInfo = {};
                set_0_layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                set_0_layoutCreateInfo.bindingCount = darray_get_length(set_0_layoutBindings);
                set_0_layoutCreateInfo.pBindings = set_0_layoutBindings;
                VkDescriptorSetLayout set_0_layout;
                if (vkCreateDescriptorSetLayout(internalStateRenderer.device, &set_0_layoutCreateInfo, NULL, &set_0_layout) != VK_SUCCESS) {
                    FATAL("Failed to create descriptor set layout");
                }

                pipeline->descriptorSetLayouts = darray_create(VkDescriptorSetLayout);
                darray_push(pipeline->descriptorSetLayouts, set_0_layout);

                PipelineOptions options = {
                    .vertShaderPath = "assets/shaders/spv/wireframe.vert.spv",
                    .fragShaderPath = "assets/shaders/spv/wireframe.frag.spv",
                    .vertexBindingDescriptions = vertexInputBindings,
                    .vertexAttributeDescriptions = vertexInputAttributeDescriptions,
                    .descriptorSetLayouts = pipeline->descriptorSetLayouts,
                    .viewport = viewport,
                    .scissor = scissor,
                    .cullMode = VK_CULL_MODE_NONE,
                    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    .polygonMode = VK_POLYGON_MODE_LINE,
                    .depthTestEnable = VK_TRUE,
                    .blendEnable = VK_FALSE,
                    .rasterizationSamples = internalStateRenderer.msaaSamples,
                    .renderPass = internalStateRenderer.renderPass,
                };
                createGraphicsPipline(internalStateRenderer.device, options, &pipeline->pipeline, &pipeline->pipelineLayout);
                createPipelineFrameUBO(internalStateRenderer, pipeline);
                TRACE("Wireframe graphics pipeline created");

                darray_destroy(vertexInputBindings);
                darray_destroy(vertexInputAttributeDescriptions);
                darray_destroy(set_0_layoutBindings);
                break;
            }
        }
}

void createPipelineFrameUBO(RendererState internalStateRenderer, PipelineState* pipeline) {
    VkDeviceSize bufferSize = sizeof(FrameUBO);

    pipeline->frameUBO = darray_create_reserve(FrameUBO, MAX_FRAMES_IN_FLIGHT);
    pipeline->frameUBOMemory = darray_create_reserve(VkDeviceMemory, MAX_FRAMES_IN_FLIGHT);
    pipeline->frameUBOMapped = darray_create_reserve(void*, MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(internalStateRenderer, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &pipeline->frameUBO[i], &pipeline->frameUBOMemory[i]);
        vkMapMemory(internalStateRenderer.device, pipeline->frameUBOMemory[i], 0, bufferSize, 0, &pipeline->frameUBOMapped[i]);
    }

    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = pipeline->descriptorSetLayouts[0];
    pipeline->descriptorSets = darray_create_reserve(VkDescriptorSet, MAX_FRAMES_IN_FLIGHT);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = internalStateRenderer.descriptorPool;
    allocInfo.pSetLayouts = layouts;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    if (vkAllocateDescriptorSets(internalStateRenderer.device, &allocInfo, pipeline->descriptorSets) != VK_SUCCESS) {
        FATAL("Failed to allocate descriptor sets");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkWriteDescriptorSet writeDescriptors[1] = {};

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = pipeline->frameUBO[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(FrameUBO);

        writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptors[0].descriptorCount = 1;
        writeDescriptors[0].dstSet = pipeline->descriptorSets[i];
        writeDescriptors[0].pBufferInfo = &bufferInfo;
        writeDescriptors[0].dstBinding = 0;
        writeDescriptors[0].dstArrayElement = 0;

        vkUpdateDescriptorSets(internalStateRenderer.device, 1, writeDescriptors, 0, NULL);
    }
}