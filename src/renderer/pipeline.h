#pragma once

#include "vulkan/vulkan.h"
#include "math_types.h"
#include "files.h"
#include "asset_types.h"

typedef struct PipelineOptions {
    const char* vertShaderPath;
    const char* fragShaderPath;
    VkVertexInputBindingDescription* vertexBindingDescriptions;
    VkVertexInputAttributeDescription* vertexAttributeDescriptions;
    VkDescriptorSetLayout descriptorSetLayout;
    VkViewport viewport;
    VkRect2D scissor;
    VkCullModeFlags cullMode;
    VkFrontFace frontFace;
    VkPolygonMode polygonMode;
    VkBool32 depthTestEnable, blendEnable;
    VkSampleCountFlagBits rasterizationSamples;
    VkRenderPass renderPass;
} PipelineOptions;

void createGraphicsPipline(VkDevice device, PipelineOptions options, VkPipeline* pipeline, VkPipelineLayout* pipelineLayout);