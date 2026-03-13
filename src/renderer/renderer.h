#pragma  once

#include "defines.h"
#include "darray.h"
#include "emath.h"
#include "platform.h"
#include "asset_types.h"
#include "rendererAPI.h"
#include "darray.h"

#include "files.h"

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

#include "pipeline.h"

typedef struct MeshRendererState {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
} MeshRendererState;

typedef struct QueueFamilyIndices {
    int32_t graphicsFamily;
    int32_t presentFamily;
    bool isComplete;
} QueueFamilyIndices;

typedef struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* presentModes;
    bool isComplete;
} SwapchainSupportDetails;

typedef struct FrameUBO {
    mat4 view;
    mat4 projection;
} FrameUBO;

typedef struct PipelineState {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet* descriptorSets;
} PipelineState;

typedef struct RendererState {
    uint32_t height, width, fpsCap;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    QueueFamilyIndices queueFamilyIndices;
    VkQueue graphicsQueue, presentQueue;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainImageExtent;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
    SwapchainSupportDetails swapchainSupportDetails;
    VkSampleCountFlagBits msaaSamples;
    VkFormat depthFormat;
    VkRenderPass renderPass;
    VkCommandPool commandPool;

    VkCommandBuffer* commandBuffers;

    VkImage colorImage;
    VkImageView colorImageView;
    VkDeviceMemory colorImageMemory;

    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;

    VkFramebuffer* framebuffers;

    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;

    uint32_t currentFrame;
    bool framebufferResized;
    bool rendererShouldDraw;
    bool rendererReady;
    double startTime;

    VkBuffer* frameUBO;
    VkDeviceMemory* frameUBOMemory;
    void** frameUBOMapped;

    PipelineState* pipelineStates;
    Entity** entityToDraw; // TODO: make thread safe

} RendererState;

void rendererInitialize();

uint32_t findMemoryType(RendererState internalRendererState, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createImage(RendererState internalRendererState, uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
void createImageView(RendererState internalRendererState, VkImageView* imageView, VkImage image, VkFormat imageFormat, uint32_t mipLevels, VkImageAspectFlags imageAspect);

VkCommandBuffer beginSingleTimeCommands(RendererState internalStateRenderer);
void endSingleTimeCommands(RendererState internalStateRenderer, VkCommandBuffer commandBuffer);
void transitionImageLayout(RendererState internalStateRenderer, VkImage image, VkFormat format, uint32_t mipLevels,VkImageLayout oldLayout, VkImageLayout newLayout);

void createBuffer(RendererState internalStateRenderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
void copyBuffer(RendererState internalStateRenderer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void createVertexBuffer(RendererState internalStateRenderer, Vertex* vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);
void createIndexBuffer(RendererState internalStateRenderer, uint32_t* indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);


PipelineState* createCommonPipelines(RendererState internalStateRenderer);