#pragma  once

#include "defines.h"
#include "darray.h"
#include "emath.h"
#include "platform.h"
#include "asset_types.h"
#include "rendererAPI.h"
#include "darray.h"

#include "utils.h"

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

typedef struct ImageRendererState {
    VkImage textureImage;
    VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;
    uint32_t mipLevels;
} ImageRendererState;

typedef struct MaterialRendererState {
    VkDescriptorSet descriptorSet;
} MaterialRendererState;

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

typedef struct PushConstant0 {
    mat4 model;
} PushConstant0;

typedef struct PipelineState {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout* descriptorSetLayouts;
    VkDescriptorSet* descriptorSets;

    VkBuffer* frameUBOBuffer;
    VkDeviceMemory* frameUBOMemory;
    void** frameUBOMapped;

    VkBuffer* lightUBOBuffer;
    VkDeviceMemory* lightUBOMemory;
    void** lightUBOMapped;
} PipelineState;

typedef struct RendererState {
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
    VkDescriptorPool descriptorPool;

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

    bool rendererReady;
    bool useWireframe;

    double startTime;
    double targetFrameTime;

    PipelineState* pipelineStates;
    VkSampler textureSampler;

    Scene* scene;
    HashMap* materialRendererStates;

} RendererState;

void rendererInitialize();

uint32_t findMemoryType(RendererState internalRendererState, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createImage(RendererState internalRendererState, uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
void createImageView(RendererState internalRendererState, VkImageView* imageView, VkImage image, VkFormat imageFormat, uint32_t mipLevels, VkImageAspectFlags imageAspect);

VkCommandBuffer beginSingleTimeCommands(RendererState internalStateRenderer);
void endSingleTimeCommands(RendererState internalStateRenderer, VkCommandBuffer commandBuffer);
void transitionImageLayout(RendererState internalStateRenderer, VkImage image, VkFormat format, uint32_t mipLevels,VkImageLayout oldLayout, VkImageLayout newLayout);
void copyBufferToImage(RendererState internalStateRenderer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void createBuffer(RendererState internalStateRenderer, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
void copyBuffer(RendererState internalStateRenderer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void createVertexBuffer(RendererState internalStateRenderer, Vertex* vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);
void createIndexBuffer(RendererState internalStateRenderer, uint32_t* indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);

void createTextureImage(RendererState internalStateRenderer, void* pixels, uint32_t width, uint32_t height, uint32_t mipLevels, VkImage* textureImage, VkImageView* textureImageView, VkDeviceMemory* textureImageMemory);
void generateMipmaps(RendererState internalStateRenderer, VkImage image, uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels);
void createTextureSampler(RendererState internalStateRenderer, VkSampler* textureSampler);

typedef struct PipelineOptions {
    const char* vertShaderPath;
    const char* fragShaderPath;
    VkVertexInputBindingDescription* vertexBindingDescriptions;
    VkVertexInputAttributeDescription* vertexAttributeDescriptions;
    VkDescriptorSetLayout* descriptorSetLayouts;
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
void createCommonPipelines(RendererState internalStateRenderer, PipelineState** pipelineStates);
void createPipelineFrameUBO(RendererState internalStateRenderer, PipelineState* pipeline);