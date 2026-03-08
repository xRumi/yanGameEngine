#pragma once

#include "defines.h"
#include "darray.h"
#include "emath.h"
#include "platform.h"

#include "files.h"

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 2

void rendererInitialize();

typedef struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} UniformBufferObject;

// Used signed integers as family indices can be 0
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

typedef struct VkState {

    double startTime;

    Vertex* vertices;
    uint32_t* indices;

    uint32_t height, width;
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
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet* descriptorSets;
    VkCommandBuffer* commandBuffers;

    VkFramebuffer* framebuffers;

    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;

    uint32_t currentFrame;
    bool framebufferResized;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage colorImage;
    VkImageView colorImageView;
    VkDeviceMemory colorImageMemory;

    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkBuffer* uniformBuffers;
    VkDeviceMemory* uniformBuffersMemory;
    void** uniformBuffersMapped;

} VkState;