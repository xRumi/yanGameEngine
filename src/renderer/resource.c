#include "renderer.h"

uint32_t findMemoryType(RendererState internalRendererState, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(internalRendererState.physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    FATAL("failed to find suitable memory type");
}

void createImage(RendererState internalRendererState, uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.format = format;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.samples = sampleCount;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(internalRendererState.device, &imageInfo, NULL, image) != VK_SUCCESS) {
        FATAL("Failed to create vulkan image");
    }

    VkMemoryRequirements memRequirements = {};
    vkGetImageMemoryRequirements(internalRendererState.device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(internalRendererState, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(internalRendererState.device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
        FATAL("Failed to allocate texture image memory");
    }
    vkBindImageMemory(internalRendererState.device, *image, *imageMemory, 0);
}

void createImageView(RendererState internalRendererState, VkImageView* imageView, VkImage image, VkFormat imageFormat, uint32_t mipLevels, VkImageAspectFlags imageAspect) {
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.image = image;
    createInfo.format = imageFormat;
    createInfo.subresourceRange.aspectMask = imageAspect;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    if (vkCreateImageView(internalRendererState.device, &createInfo, NULL, imageView) != VK_SUCCESS) {
        FATAL("Failed to create image view");
    }
}

VkCommandBuffer beginSingleTimeCommands(RendererState internalStateRenderer) {
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = internalStateRenderer.commandPool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    if (vkAllocateCommandBuffers(internalStateRenderer.device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        FATAL("Failed to allocate single time command buffer");
    }
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void endSingleTimeCommands(RendererState internalStateRenderer, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(internalStateRenderer.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkDeviceWaitIdle(internalStateRenderer.device);
    vkFreeCommandBuffers(internalStateRenderer.device, internalStateRenderer.commandPool, 1, &commandBuffer);
}
void transitionImageLayout(RendererState internalStateRenderer, VkImage image, VkFormat format, uint32_t mipLevels,VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(internalStateRenderer);
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    } else barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        FATAL("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);
    endSingleTimeCommands(internalStateRenderer, commandBuffer);
}


void createBuffer(RendererState internalStateRenderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(internalStateRenderer.device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        FATAL("Failed to create vulkan buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(internalStateRenderer.device, *buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(internalStateRenderer, memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(internalStateRenderer.device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        FATAL("Failed to allocate vulkan buffer memory");
    };
    vkBindBufferMemory(internalStateRenderer.device, *buffer, *bufferMemory, 0);
}

void copyBuffer(RendererState internalStateRenderer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(internalStateRenderer);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(internalStateRenderer, commandBuffer);
}

void createVertexBuffer(RendererState internalStateRenderer, Vertex* vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory) {
    VkDeviceSize bufferSize = darray_get_length(vertices) * sizeof(Vertex);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(internalStateRenderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    void* data;
    vkMapMemory(internalStateRenderer.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(internalStateRenderer.device, stagingBufferMemory);
    createBuffer(internalStateRenderer, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    copyBuffer(internalStateRenderer, stagingBuffer, *vertexBuffer, bufferSize);
    vkDestroyBuffer(internalStateRenderer.device, stagingBuffer, NULL);
    vkFreeMemory(internalStateRenderer.device, stagingBufferMemory, NULL);
}

void createIndexBuffer(RendererState internalStateRenderer, uint32_t* indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory) {
    VkDeviceSize bufferSize = darray_get_length(indices) * sizeof(indices[0]);
    VkBuffer staggingBuffer;
    VkDeviceMemory staggingMemory;
    createBuffer(internalStateRenderer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staggingBuffer, &staggingMemory);
    void *data;
    vkMapMemory(internalStateRenderer.device, staggingMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(internalStateRenderer.device, staggingMemory);
    createBuffer(internalStateRenderer, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
    copyBuffer(internalStateRenderer, staggingBuffer, *indexBuffer, bufferSize);
    vkDestroyBuffer(internalStateRenderer.device, staggingBuffer, NULL);
    vkFreeMemory(internalStateRenderer.device, staggingMemory, NULL);
}

PipelineState* createCommonPipelines(RendererState internalStateRenderer) {
    PipelineState* pipelineStates = darray_create_reserve(PipelineState, PIPELINE_TYPE_MAX);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = (float)internalStateRenderer.height;
    viewport.width = (float)internalStateRenderer.width;
    viewport.height = -(float)internalStateRenderer.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = (VkExtent2D){.width = internalStateRenderer.width, .height = internalStateRenderer.height};

    for (int i = 0; i < PIPELINE_TYPE_MAX; i++)
        switch (i) {
            case PIPELINE_TYPE_MESH: {
                // create mesh pipeline

                PipelineState* pipeline = &pipelineStates[i];

                VkVertexInputBindingDescription* vertexInputBindings = darray_create_reserve(VkVertexInputBindingDescription, 1);
                vertexInputBindings[0].binding = 0;
                vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                vertexInputBindings[0].stride = sizeof(Vertex);

                VkVertexInputAttributeDescription* vertexInputAttributeDescriptions = darray_create_reserve(VkVertexInputAttributeDescription, 3);
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

                VkDescriptorSetLayoutBinding* descriptorSetLayoutBindings = darray_create_reserve(VkDescriptorSetLayoutBinding, 1);

                descriptorSetLayoutBindings[0].binding = 0;
                descriptorSetLayoutBindings[0].descriptorCount = 1;
                descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
                descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                descriptorSetLayoutCreateInfo.bindingCount = darray_get_length(descriptorSetLayoutBindings);
                descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;
                if (vkCreateDescriptorSetLayout(internalStateRenderer.device, &descriptorSetLayoutCreateInfo, NULL, &pipeline->descriptorSetLayout) != VK_SUCCESS) {
                    FATAL("Failed to create descriptor set layout");
                }

                PipelineOptions options = {
                    .vertShaderPath = "assets/shaders/mesh.vert.spv",
                    .fragShaderPath = "assets/shaders/mesh.frag.spv",
                    .vertexBindingDescriptions = vertexInputBindings,
                    .vertexAttributeDescriptions = vertexInputAttributeDescriptions,
                    .descriptorSetLayout = pipeline->descriptorSetLayout,
                    .viewport = viewport,
                    .scissor = scissor,
                    .cullMode = VK_CULL_MODE_BACK_BIT,
                    .frontFace = VK_FRONT_FACE_CLOCKWISE,
                    .polygonMode = VK_POLYGON_MODE_FILL,
                    .depthTestEnable = VK_TRUE,
                    .blendEnable = VK_FALSE,
                    .rasterizationSamples = internalStateRenderer.msaaSamples,
                    .renderPass = internalStateRenderer.renderPass,
                };
                createGraphicsPipline(internalStateRenderer.device, options, &pipeline->pipeline, &pipeline->pipelineLayout);
                TRACE("Mesh graphics pipeline created");

                darray_destroy(vertexInputBindings);
                darray_destroy(vertexInputAttributeDescriptions);
                darray_destroy(descriptorSetLayoutBindings);

                break;
            }
        }
    return pipelineStates;
}