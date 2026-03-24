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

void copyBufferToImage(RendererState internalStateRenderer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(internalStateRenderer);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){
        width,
        height,
        1
    };
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    endSingleTimeCommands(internalStateRenderer, commandBuffer);
}

void generateMipmaps(RendererState internalStateRenderer, VkImage image, uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels) {
    VkFormatProperties formatProperties = {};
    vkGetPhysicalDeviceFormatProperties(internalStateRenderer.physicalDevice, format, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        FATAL("Texture image format does not support linear filter");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(internalStateRenderer);
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    
    int32_t mipWidth = width, mipHeight = height;
    
    for (uint32_t i = 1; i < mipLevels; i++) {
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);

        VkImageBlit imageBlit = {};
        imageBlit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
        imageBlit.srcOffsets[1] = (VkOffset3D){mipWidth, mipHeight, 1};
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.mipLevel = i - 1;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.baseArrayLayer = 0;
        imageBlit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
        imageBlit.dstOffsets[1] = (VkOffset3D){mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.mipLevel = i;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.baseArrayLayer = 0;
        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);

    endSingleTimeCommands(internalStateRenderer, commandBuffer);
}

void createTextureImage(RendererState internalStateRenderer, void* pixels, uint32_t width, uint32_t height, uint32_t mipLevels, VkImage* textureImage, VkImageView* textureImageView, VkDeviceMemory* textureImageMemory) {
    VkDeviceSize imageSize = width * height * 4;
    VkBuffer staggingBuffer;
    VkDeviceMemory staggingMemory;
    createBuffer(internalStateRenderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staggingBuffer, &staggingMemory);
    
    void* data;
    vkMapMemory(internalStateRenderer.device, staggingMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(internalStateRenderer.device, staggingMemory);

    createImage(internalStateRenderer, width, height, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
    transitionImageLayout(internalStateRenderer, *textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(internalStateRenderer, staggingBuffer, *textureImage, width, height);
    // transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    generateMipmaps(internalStateRenderer, *textureImage, width, height, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);

    vkDestroyBuffer(internalStateRenderer.device, staggingBuffer, NULL);
    vkFreeMemory(internalStateRenderer.device, staggingMemory, NULL);

    createImageView(internalStateRenderer, textureImageView, *textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_IMAGE_ASPECT_COLOR_BIT);
}

void createTextureSampler(RendererState internalStateRenderer, VkSampler* textureSampler) {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(internalStateRenderer.physicalDevice, &properties);

    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.anisotropyEnable = VK_TRUE;
    createInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.maxLod = VK_LOD_CLAMP_NONE;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    if (vkCreateSampler(internalStateRenderer.device, &createInfo, NULL, textureSampler) != VK_SUCCESS) {
        FATAL("Failed to create texture sampler");
    }
}