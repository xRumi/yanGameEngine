#include "pipeline.h"

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

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &options.descriptorSetLayout;

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