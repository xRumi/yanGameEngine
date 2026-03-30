#include "renderer.h"

const char* instanceLayers[] = {"VK_LAYER_KHRONOS_validation"};
const char* instanceExtensions[] = {VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME};
const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME};

extern int platformWindowClosed;
RendererState internalStateRenderer;

void createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "vulkanEngine";
    appInfo.pEngineName = "vulkanEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = sizeof(instanceLayers) / sizeof(instanceLayers[0]);
    createInfo.ppEnabledLayerNames = instanceLayers;
    createInfo.enabledExtensionCount = sizeof(instanceExtensions) / sizeof(instanceExtensions[0]);
    createInfo.ppEnabledExtensionNames = instanceExtensions;
    
    if (vkCreateInstance(&createInfo, NULL, &internalStateRenderer.instance) != VK_SUCCESS) {
        FATAL("Failed to create vk instance");
    }
}

void createSurface() {
#ifdef __linux__
    VkWaylandSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.surface = platformGetPlatformState()->surface;
    createInfo.display = platformGetPlatformState()->display;

    if (vkCreateWaylandSurfaceKHR(internalStateRenderer.instance, &createInfo, NULL, &internalStateRenderer.surface) != VK_SUCCESS) {
        FATAL("Failed to create vulkan wayland surface");
    }
#elif
    FATAL("Failed to create vulkan surface, platform not implemented");
#endif
}

QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device) {
    QueueFamilyIndices familyIndices = {-1, -1, false};
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilyProperties = darray_create_reserve(VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties);
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (darray_at_type(queueFamilyProperties, i, VkQueueFamilyProperties).queueFlags & VK_QUEUE_GRAPHICS_BIT) familyIndices.graphicsFamily = i;
        VkBool32 presentFamily = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, internalStateRenderer.surface, &presentFamily);
        if (presentFamily) familyIndices.presentFamily = i;
        if (familyIndices.graphicsFamily >= 0 && familyIndices.presentFamily >= 0) {
            familyIndices.isComplete = true;
            break;
        }
    }
    darray_destroy(queueFamilyProperties);
    return familyIndices;
}

SwapchainSupportDetails findSwapchainSupportDetails(VkPhysicalDevice device) {
    SwapchainSupportDetails details = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, internalStateRenderer.surface, &details.capabilities);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, internalStateRenderer.surface, &formatCount, NULL);
    details.formats = darray_create_reserve(VkSurfaceFormatKHR, formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, internalStateRenderer.surface, &formatCount, details.formats);
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, internalStateRenderer.surface, &presentModeCount, NULL);
    details.presentModes = darray_create_reserve(VkSurfacePresentModeKHR, presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, internalStateRenderer.surface, &presentModeCount, details.presentModes);
    if (formatCount > 0 && presentModeCount > 0) details.isComplete = true;

    return details;
}

VkSampleCountFlagBits getMaxUsableSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(internalStateRenderer.physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

void pickPhysicalDevice() {
    // Get physical devices
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(internalStateRenderer.instance, &physicalDeviceCount, NULL);
    VkPhysicalDevice* physicalDevices = darray_create_reserve(VkPhysicalDevice, physicalDeviceCount);
    vkEnumeratePhysicalDevices(internalStateRenderer.instance, &physicalDeviceCount, physicalDevices);

    // Find the best GPU
    TRACE("Found %ld physical devices:", physicalDeviceCount);
    uint32_t maxScore = 0;
    VkPhysicalDevice pickedDevice = VK_NULL_HANDLE;
    const char* pickedDeviceName = NULL;
    SwapchainSupportDetails pickedSwapchainDetails;

    QueueFamilyIndices pickedQueueFamily;
    for (uint32_t i = 0; i < physicalDeviceCount; i++) {
        VkPhysicalDevice device = darray_at_type(physicalDevices, i, VkPhysicalDevice);
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceFeatures(device, &features);
        TRACE("@\t%s", properties.deviceName);

        QueueFamilyIndices familyIndices = findQueueFamilyIndices(device);
        if (!familyIndices.isComplete) continue;

        // check for required device extension support
        uint32_t deviceExtensionsCount;
        vkEnumerateDeviceExtensionProperties(device, NULL, &deviceExtensionsCount, NULL);
        VkExtensionProperties* extensions = darray_create_reserve(VkExtensionProperties, deviceExtensionsCount);
        vkEnumerateDeviceExtensionProperties(device, NULL, &deviceExtensionsCount, extensions);
        uint32_t requiredDeviceExtensionsCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
        bool allExtensionFound = true;
        for (uint32_t i = 0; i < requiredDeviceExtensionsCount; i++) {
            bool found = false;
            for (uint32_t j = 0; j < deviceExtensionsCount; j++)
                if (strcmp(deviceExtensions[i], darray_at_type(extensions, j, VkExtensionProperties).extensionName) == 0) {
                    found = true;
                }
            if (!found) {
                allExtensionFound = false; 
            }
        }
        darray_destroy(extensions);
        if (!allExtensionFound) continue;

        SwapchainSupportDetails swapchainDetails = findSwapchainSupportDetails(device);
        if (!swapchainDetails.isComplete) {
            darray_destroy(swapchainDetails.formats);
            darray_destroy(swapchainDetails.presentModes);
            continue;
        }

        int score = properties.limits.maxImageDimension2D;
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
        if (score > maxScore) {
            pickedDevice = device;
            pickedDeviceName = properties.deviceName;
            pickedQueueFamily = familyIndices;
            pickedSwapchainDetails = swapchainDetails;
            maxScore = score;
        }
    }
    darray_destroy(physicalDevices);

    if (pickedDevice != VK_NULL_HANDLE) {
        TRACE("Physical Device \"%s\" selected", pickedDeviceName);
        internalStateRenderer.physicalDevice = pickedDevice;
        internalStateRenderer.queueFamilyIndices = pickedQueueFamily;
        internalStateRenderer.swapchainSupportDetails = pickedSwapchainDetails;
        internalStateRenderer.msaaSamples = getMaxUsableSampleCount();
    } else {
        FATAL("Failed to select Physical Device");
    }
}

void createLogicalDevice() {
    const float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo* queueCreateInfos = darray_create(VkDeviceQueueCreateInfo);

    VkDeviceQueueCreateInfo graphicsFamilyQueue = {};
    graphicsFamilyQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicsFamilyQueue.queueCount = 1;
    graphicsFamilyQueue.queueFamilyIndex = internalStateRenderer.queueFamilyIndices.graphicsFamily;
    graphicsFamilyQueue.pQueuePriorities = &queuePriority;

    darray_push(queueCreateInfos, graphicsFamilyQueue);

    if (internalStateRenderer.queueFamilyIndices.graphicsFamily != internalStateRenderer.queueFamilyIndices.presentFamily) {
        VkDeviceQueueCreateInfo presentFamilyQueue = {};
        presentFamilyQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentFamilyQueue.queueCount = 1;
        presentFamilyQueue.queueFamilyIndex = internalStateRenderer.queueFamilyIndices.presentFamily;
        presentFamilyQueue.pQueuePriorities = &queuePriority;
        darray_push(queueCreateInfos, presentFamilyQueue);
    }

    VkPhysicalDeviceFeatures features = {};
    features.samplerAnisotropy = true;
    features.fillModeNonSolid = true;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = darray_get_length(queueCreateInfos);
    createInfo.ppEnabledLayerNames = instanceLayers;
    createInfo.enabledLayerCount = sizeof(instanceLayers) / sizeof(instanceLayers[0]);
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    createInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
    createInfo.pEnabledFeatures = &features;
    if (vkCreateDevice(internalStateRenderer.physicalDevice, &createInfo, NULL, &internalStateRenderer.device) != VK_SUCCESS) {
        FATAL("Failed to create vulkan device");
    }
    darray_destroy(queueCreateInfos);
    vkGetDeviceQueue(internalStateRenderer.device, internalStateRenderer.queueFamilyIndices.graphicsFamily, 0, &internalStateRenderer.graphicsQueue);
    vkGetDeviceQueue(internalStateRenderer.device, internalStateRenderer.queueFamilyIndices.presentFamily, 0, &internalStateRenderer.presentQueue);
}

void createSwapchain() {
    // Select format and presentMode from available formats and present modes
    VkSurfaceFormatKHR format = internalStateRenderer.swapchainSupportDetails.formats[0];
    for (uint32_t i = 0; i < darray_get_length(internalStateRenderer.swapchainSupportDetails.formats); i++) {
        if (darray_at_type(internalStateRenderer.swapchainSupportDetails.formats, i, VkSurfaceFormatKHR).format == VK_FORMAT_B8G8R8A8_SRGB && darray_at_type(internalStateRenderer.swapchainSupportDetails.formats, i, VkSurfaceFormatKHR).colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            format = darray_at_type(internalStateRenderer.swapchainSupportDetails.formats, i, VkSurfaceFormatKHR);
            break;
        }
    }
    // Every device that have present queue got support for FIFO present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < darray_get_length(internalStateRenderer.swapchainSupportDetails.presentModes); i++) {
        if (darray_at_type(internalStateRenderer.swapchainSupportDetails.presentModes, i, VkPresentModeKHR) == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    VkExtent2D extent = internalStateRenderer.swapchainSupportDetails.capabilities.currentExtent;
    extent.height = platformGetPlatformState()->height;
    extent.width = platformGetPlatformState()->width;

    uint32_t imageCount = internalStateRenderer.swapchainSupportDetails.capabilities.minImageCount + 1;
    if (internalStateRenderer.swapchainSupportDetails.capabilities.maxImageCount != 0 && imageCount > internalStateRenderer.swapchainSupportDetails.capabilities.maxImageCount) {
        imageCount = internalStateRenderer.swapchainSupportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = internalStateRenderer.surface;
    createInfo.presentMode = presentMode;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.minImageCount = imageCount;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = internalStateRenderer.swapchainSupportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (internalStateRenderer.queueFamilyIndices.graphicsFamily == internalStateRenderer.queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    } else {
        uint32_t indices[] = {internalStateRenderer.queueFamilyIndices.graphicsFamily, internalStateRenderer.queueFamilyIndices.presentFamily};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }

    if (vkCreateSwapchainKHR(internalStateRenderer.device, &createInfo, NULL, &internalStateRenderer.swapchain) != VK_SUCCESS) {
        FATAL("Failed to create swapchain");
    }

    // get swapchain images and create image views for them
    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(internalStateRenderer.device, internalStateRenderer.swapchain, &swapchainImageCount, NULL);
    internalStateRenderer.swapchainImages = darray_create_reserve(VkImage, swapchainImageCount);
    vkGetSwapchainImagesKHR(internalStateRenderer.device, internalStateRenderer.swapchain, &swapchainImageCount, internalStateRenderer.swapchainImages);
    TRACE("Swapchain image count: %d", swapchainImageCount);
    
    internalStateRenderer.swapchainImageFormat = format.format;
    internalStateRenderer.swapchainImageExtent = extent;
    internalStateRenderer.swapchainImageViews = darray_create_reserve(VkImageView, swapchainImageCount);
    for (uint32_t i = 0; i < swapchainImageCount; i++)
        createImageView(internalStateRenderer, &internalStateRenderer.swapchainImageViews[i], internalStateRenderer.swapchainImages[i], internalStateRenderer.swapchainImageFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkFormat findSupportedFormat(VkFormat* candidates, uint32_t formatCount, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (int32_t i = 0; i < formatCount; i++) {
        VkFormat format = candidates[i];
        VkFormatProperties properties = {};
        vkGetPhysicalDeviceFormatProperties(internalStateRenderer.physicalDevice, format, &properties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) return format;
    }
    FATAL("Failed to find supported format");
}

void createRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = internalStateRenderer.swapchainImageFormat;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.samples = internalStateRenderer.msaaSamples;

    VkFormat formatChoice[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    internalStateRenderer.depthFormat = findSupportedFormat(formatChoice, sizeof(formatChoice) / sizeof(formatChoice[0]), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = internalStateRenderer.depthFormat;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.samples = internalStateRenderer.msaaSamples;

    VkAttachmentDescription colorAttachmentResolve = {};
    colorAttachmentResolve.format = internalStateRenderer.swapchainImageFormat;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef = {};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstSubpass = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment, colorAttachmentResolve};

    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(internalStateRenderer.device, &createInfo, NULL, &internalStateRenderer.renderPass) != VK_SUCCESS) {
        FATAL("Failed to create renderPass");
    }
}

void createCommandPool() {
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = internalStateRenderer.queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(internalStateRenderer.device, &createInfo, NULL, &internalStateRenderer.commandPool) != VK_SUCCESS) {
        FATAL("Failed to create command pool");
    }
}

void createCommandBuffers() {
    internalStateRenderer.commandBuffers = darray_create_reserve(VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.commandPool = internalStateRenderer.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    if (vkAllocateCommandBuffers(internalStateRenderer.device, &allocInfo, internalStateRenderer.commandBuffers) != VK_SUCCESS) {
        FATAL("Failed to allocate command buffer");
    }
}

void createColorResources() {
    createImage(internalStateRenderer, internalStateRenderer.swapchainImageExtent.width, internalStateRenderer.swapchainImageExtent.height, internalStateRenderer.swapchainImageFormat, 1, internalStateRenderer.msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &internalStateRenderer.colorImage, &internalStateRenderer.colorImageMemory);
    createImageView(internalStateRenderer, &internalStateRenderer.colorImageView, internalStateRenderer.colorImage, internalStateRenderer.swapchainImageFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT);
}

void createDepthResources() {
    VkFormat depthFormat = internalStateRenderer.depthFormat;
    createImage(internalStateRenderer, internalStateRenderer.swapchainImageExtent.width, internalStateRenderer.swapchainImageExtent.height, depthFormat, 1, internalStateRenderer.msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &internalStateRenderer.depthImage, &internalStateRenderer.depthImageMemory);
    createImageView(internalStateRenderer, &internalStateRenderer.depthImageView, internalStateRenderer.depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
    transitionImageLayout(internalStateRenderer, internalStateRenderer.depthImage, depthFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void createFramebuffers() {
    internalStateRenderer.framebuffers = darray_create_reserve(VkFramebuffer, darray_get_length(internalStateRenderer.swapchainImages));
    for (uint32_t i = 0; i < darray_get_length(internalStateRenderer.swapchainImages); i++) {
        VkImageView attachments[] = {internalStateRenderer.colorImageView, internalStateRenderer.depthImageView, internalStateRenderer.swapchainImageViews[i]};
        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = internalStateRenderer.renderPass;
        createInfo.attachmentCount = CARRAY_SIZE(attachments);
        createInfo.pAttachments = attachments;
        createInfo.width = internalStateRenderer.swapchainImageExtent.width;
        createInfo.height = internalStateRenderer.swapchainImageExtent.height;
        createInfo.layers = 1;
        
        if (vkCreateFramebuffer(internalStateRenderer.device, &createInfo, NULL, &internalStateRenderer.framebuffers[i]) != VK_SUCCESS) {
            FATAL("Failed to create framebuffer");
        }
    }
}

void createSyncObjects() {
    internalStateRenderer.imageAvailableSemaphores = darray_create_reserve(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    internalStateRenderer.renderFinishedSemaphores = darray_create_reserve(VkSemaphore, darray_get_length(internalStateRenderer.swapchainImageViews));
    internalStateRenderer.inFlightFences = darray_create_reserve(VkFence, MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(internalStateRenderer.device, &semaphoreCreateInfo, NULL, &internalStateRenderer.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(internalStateRenderer.device, &fenceCreateInfo, NULL, &internalStateRenderer.inFlightFences[i]) != VK_SUCCESS) {
            FATAL("Failed to create sync objects");
        }
    }

    for (int i = 0; i < darray_get_length(internalStateRenderer.swapchainImageViews); i++) {
        if (vkCreateSemaphore(internalStateRenderer.device, &semaphoreCreateInfo, NULL, &internalStateRenderer.renderFinishedSemaphores[i])) {
            FATAL("Failed to create sync objects");
        }
    }
}

void createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT * PIPELINE_TYPE_MAX;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT * PIPELINE_TYPE_MAX;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = CARRAY_SIZE(poolSizes);
    descriptorPoolCreateInfo.pPoolSizes = poolSizes;
    descriptorPoolCreateInfo.maxSets = 1000;
    if (vkCreateDescriptorPool(internalStateRenderer.device, &descriptorPoolCreateInfo, NULL, &internalStateRenderer.descriptorPool) != VK_SUCCESS) {
        FATAL("Failed to create descriptor pool");
    }
}

void updateFrameUBO(PipelineState* pipelineState, double deltaTime) {
    double elapsedTime = platformGetTime() - internalStateRenderer.startTime;
    (void)elapsedTime;

    if (internalStateRenderer.camera.dirty) {
        internalStateRenderer.camera.dirty = true;

        uint32_t sensitivity = 200;
        if (platformWindowIsFocused() && platformPointerIsLocked()) {
            PointerInput pointerRelative = platformInputPointerRelative();
            // rotation(euler angle): x = yaw, y = pitch, z = roll
            internalStateRenderer.camera.rotation.x += -pointerRelative.x / (float)platformGetPlatformState()->width * sensitivity * deltaTime;
            internalStateRenderer.camera.rotation.y += -pointerRelative.y / (float)platformGetPlatformState()->height * sensitivity * deltaTime;
            internalStateRenderer.camera.rotation.x = fmodf(internalStateRenderer.camera.rotation.x, TO_RADIANS(360));
            internalStateRenderer.camera.rotation.y = clamp(internalStateRenderer.camera.rotation.y, -1.5f, 1.5);
        }

        // vec3 front = mat4_mul_vec3(mat4_mul(mat4_rotation_y(TO_DEGREE(internalStateRenderer.camera.rotation.x)), mat4_rotation_x(TO_DEGREE(internalStateRenderer.camera.rotation.y))), FRONT_DIRECTION_EULER_ANGLE_YXZ_VEC3);
        vec3 front = vec3_normalize((vec3){{-sinf(internalStateRenderer.camera.rotation.x)*cosf(internalStateRenderer.camera.rotation.y), sinf(internalStateRenderer.camera.rotation.y), -cosf(internalStateRenderer.camera.rotation.x)*cosf(internalStateRenderer.camera.rotation.y)}});

        uint32_t cameraMoveSpeed = 10;
        vec3 forwardMoveAmount = vec3_scale(front, deltaTime * cameraMoveSpeed);
        vec3 rightMoveAmount = vec3_scale(vec3_normalize(vec3_cross(front, UP_DIRECTION_VEC3)), deltaTime * cameraMoveSpeed);
        if (platformInputIsKeyDown(KEY_w)) internalStateRenderer.camera.position = vec3_add(internalStateRenderer.camera.position, forwardMoveAmount);
        if (platformInputIsKeyDown(KEY_s)) internalStateRenderer.camera.position = vec3_add(internalStateRenderer.camera.position, vec3_neg(forwardMoveAmount));
        if (platformInputIsKeyDown(KEY_a)) internalStateRenderer.camera.position = vec3_add(internalStateRenderer.camera.position, vec3_neg(rightMoveAmount));
        if (platformInputIsKeyDown(KEY_d)) internalStateRenderer.camera.position = vec3_add(internalStateRenderer.camera.position, rightMoveAmount);
        if (platformInputIsKeyDown(KEY_q)) internalStateRenderer.camera.position.y += deltaTime * cameraMoveSpeed;
        if (platformInputIsKeyDown(KEY_e)) internalStateRenderer.camera.position.y -= deltaTime * cameraMoveSpeed;

        internalStateRenderer.camera.view = mat4_view_YXZ(internalStateRenderer.camera.position, internalStateRenderer.camera.rotation);
        internalStateRenderer.camera.projection = mat4_perspective(45, (float)platformGetPlatformState()->width / (float)platformGetPlatformState()->height, 0.1f, 1000.0f);
    }
    FrameUBO ubo = {
        .view = internalStateRenderer.camera.view,
        .projection = internalStateRenderer.camera.projection
    };
    memcpy(pipelineState->frameUBOMapped[internalStateRenderer.currentFrame], &ubo, sizeof(ubo));
}

void rendererLoadMesh(Mesh* mesh) {
    MeshRendererState* meshRendererState = memalloc(sizeof(MeshRendererState), MEMORY_TAG_RENDERER);
    createVertexBuffer(internalStateRenderer, mesh->vertices, &meshRendererState->vertexBuffer, &meshRendererState->vertexBufferMemory);
    createIndexBuffer(internalStateRenderer, mesh->indices, &meshRendererState->indexBuffer, &meshRendererState->indexBufferMemory);
    mesh->meshRendererStateRef = meshRendererState;
    TRACE("Load mesh");
}
void rendererLoadImage(Image* image) {
    ImageRendererState* imageRendererState = memalloc(sizeof(ImageRendererState), MEMORY_TAG_RENDERER);
    imageRendererState->mipLevels = floor(log2(MAX(image->width, image->height))) + 1;
    createTextureImage(internalStateRenderer, image->data, image->width, image->height, imageRendererState->mipLevels, &imageRendererState->textureImage, &imageRendererState->textureImageView, &imageRendererState->textureImageMemory);
    image->imageRendererStateRef = imageRendererState;
}
void rendererLoadMaterial(Material* material, HashMap* images) {
    MaterialRendererState* materialRendererState = memalloc(sizeof(MaterialRendererState), MEMORY_TAG_RENDERER);
    PipelineState* pipeline = &internalStateRenderer.pipelineStates[PIPELINE_TYPE_DEFAULT];
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = internalStateRenderer.descriptorPool;
    allocInfo.pSetLayouts = &pipeline->descriptorSetLayouts[1];
    allocInfo.descriptorSetCount = 1;
    VkResult f = vkAllocateDescriptorSets(internalStateRenderer.device, &allocInfo, &materialRendererState->descriptorSet);
    if (f != VK_SUCCESS) {
        FATAL("Failed to allocate material descriptor sets");
    }
    VkWriteDescriptorSet writeDescriptors[1] = {};

    {
        ImageRendererState* imageRendererState = material->baseColor.image->imageRendererStateRef;
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageRendererState->textureImageView;
        imageInfo.sampler = internalStateRenderer.textureSampler;

        writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptors[0].descriptorCount = 1;
        writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptors[0].dstSet = materialRendererState->descriptorSet;
        writeDescriptors[0].pImageInfo = &imageInfo;
        writeDescriptors[0].dstBinding = 0;
        writeDescriptors[0].dstArrayElement = 0;
    }

    vkUpdateDescriptorSets(internalStateRenderer.device, CARRAY_SIZE(writeDescriptors), writeDescriptors, 0, NULL);

    material->materialRendererStateRef = materialRendererState;
}
void rendererLoadModel(Model* model) {
    if (model->rendererLoaded) return;
    Image* image;
    hashmap_foreach(model->images, image) {
        rendererLoadImage(image);
    }
    Material* material;
    hashmap_foreach(model->materials, material) {
        rendererLoadMaterial(material, model->images);
        int meshCount = darray_get_length(material->meshes);
        for (int i = 0; i < meshCount; i++) {
            Mesh* mesh = &material->meshes[i];
            rendererLoadMesh(mesh);
        }
    }
    model->rendererLoaded = true;
    TRACE("Load model");
}

void recordCommandBuffer(const VkCommandBuffer commandBuffer, uint32_t imageIndex, double deltaTime) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        FATAL("Failed to begin command buffer");
    }
    VkClearValue clearColors[2] = {
        {.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
        {.depthStencil = {1.0f, 0}}
    };
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = internalStateRenderer.renderPass;
    renderPassInfo.framebuffer = internalStateRenderer.framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = internalStateRenderer.swapchainImageExtent;
    renderPassInfo.clearValueCount = CARRAY_SIZE(clearColors);
    renderPassInfo.pClearValues = clearColors;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)platformGetPlatformState()->width;
    viewport.height = (float)platformGetPlatformState()->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = (VkOffset2D){};
    scissor.extent = internalStateRenderer.swapchainImageExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    enum PipelineType previousPipelineType = PIPELINE_TYPE_MAX;
    Entity* entity;
    hashmap_foreach(internalStateRenderer.entities, entity) {
        Model* model = entity->model;
        if (!model->rendererLoaded) rendererLoadModel(model);

        PushConstant0 pushConstant0 = {};
        pushConstant0.model = entity->transform;

        Material* material;
        hashmap_foreach(model->materials, material) {
            MaterialRendererState* materialRendererState = (MaterialRendererState*)material->materialRendererStateRef;
            enum PipelineType currentPipelineType = material->pipelineType;
            if (internalStateRenderer.useWireframe) currentPipelineType = PIPELINE_TYPE_WIREFRAME;
            PipelineState pipelineState = internalStateRenderer.pipelineStates[currentPipelineType];
            if (previousPipelineType != currentPipelineType) {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineState.pipeline);
                updateFrameUBO(&pipelineState, deltaTime);
                previousPipelineType = currentPipelineType;
            }
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineState.pipelineLayout, 0, 1, &pipelineState.descriptorSets[internalStateRenderer.currentFrame], 0, NULL);
            vkCmdPushConstants(commandBuffer, pipelineState.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstant0), &pushConstant0);

            int meshCount = darray_get_length(material->meshes);
            for (int j = 0; j < meshCount; j++) {
                Mesh mesh = material->meshes[j];
                MeshRendererState* meshRendererState = mesh.meshRendererStateRef;

                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshRendererState->vertexBuffer, offsets);
                vkCmdBindIndexBuffer(commandBuffer, meshRendererState->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                switch (currentPipelineType) {
                    case PIPELINE_TYPE_DEFAULT: {
                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineState.pipelineLayout, 1, 1, &materialRendererState->descriptorSet, 0, NULL);
                        break;
                    }
                    case PIPELINE_TYPE_WIREFRAME:
                    case PIPELINE_TYPE_MAX: break;
                }
                vkCmdDrawIndexed(commandBuffer, darray_get_length(mesh.indices), 1, 0, 0, 0);
            }
        }
    }
    vkCmdEndRenderPass(commandBuffer);
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        FATAL("Failed to record command buffer");
    }
}

void drawFrame(double deltaTime) {
    vkWaitForFences(internalStateRenderer.device, 1, &internalStateRenderer.inFlightFences[internalStateRenderer.currentFrame], VK_TRUE, UINT32_MAX);
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(internalStateRenderer.device, internalStateRenderer.swapchain, UINT32_MAX, internalStateRenderer.imageAvailableSemaphores[internalStateRenderer.currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // recreateSwapchain();
        WARN("Need to recreate swapchain but not implemented");
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        FATAL("Failed to acquire next swapchain image");
    }
    vkResetFences(internalStateRenderer.device, 1, &internalStateRenderer.inFlightFences[internalStateRenderer.currentFrame]);
    vkResetCommandBuffer(internalStateRenderer.commandBuffers[internalStateRenderer.currentFrame], 0);
    recordCommandBuffer(internalStateRenderer.commandBuffers[internalStateRenderer.currentFrame], imageIndex, deltaTime);
    VkPipelineStageFlags waitStage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &internalStateRenderer.commandBuffers[internalStateRenderer.currentFrame];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &internalStateRenderer.imageAvailableSemaphores[internalStateRenderer.currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &internalStateRenderer.renderFinishedSemaphores[imageIndex];
    submitInfo.pWaitDstStageMask = waitStage;
    
    if (vkQueueSubmit(internalStateRenderer.graphicsQueue, 1, &submitInfo, internalStateRenderer.inFlightFences[internalStateRenderer.currentFrame]) != VK_SUCCESS) {
        FATAL("Failed to submit draw command to queue");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &internalStateRenderer.renderFinishedSemaphores[imageIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &internalStateRenderer.swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    result = vkQueuePresentKHR(internalStateRenderer.presentQueue, &presentInfo);
    if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || internalStateRenderer.framebufferResized) {
        internalStateRenderer.framebufferResized = false;
        // recreateSwapchain();
        WARN("Need to recreate swapchain but not implemented");
        return;
    } else if (result != VK_SUCCESS) {
        FATAL("Failed to present swapchain image");
    }
    internalStateRenderer.currentFrame = (internalStateRenderer.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void mainLoop() {
    double deltaTime = 0;
    double currentTime, lastTime = platformGetTime();

    while (!platformGetPlatformState()->platformWindowClosed) {
        currentTime = platformGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        drawFrame(deltaTime);
    
        double frameTime = platformGetTime() - currentTime;
        if (frameTime < internalStateRenderer.targetFrameTime) {
            double sleepTime = internalStateRenderer.targetFrameTime - frameTime;
            platformSleep(sleepTime);
        }
    }
}

void vulkanRendererInitialize() {
    platformGetPlatformState()->width = platformGetPlatformState()->width;
    platformGetPlatformState()->height = platformGetPlatformState()->height;
    internalStateRenderer.startTime = platformGetTime();
    internalStateRenderer.targetFrameTime = 1 / 60.0;
    internalStateRenderer.entities = hashmap_create(1000);
    internalStateRenderer.materialRendererStates = hashmap_create(100);
    rendererCameraReset();

    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createRenderPass();
    createCommandPool();
    createCommandBuffers();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
    createDescriptorPool();

    createCommonPipelines(internalStateRenderer, &internalStateRenderer.pipelineStates);
    createTextureSampler(internalStateRenderer, &internalStateRenderer.textureSampler);
}

void vulkanCleanUp() {
    vkDeviceWaitIdle(internalStateRenderer.device);

    for (int i = 0; i < darray_get_length(internalStateRenderer.swapchainImages); i++) {
        vkDestroySemaphore(internalStateRenderer.device, internalStateRenderer.renderFinishedSemaphores[i], NULL);
    }

    // for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    //     vkDestroyBuffer(internalStateRenderer.device, internalStateRenderer.uniformBuffers[i], NULL);
    //     vkUnmapMemory(internalStateRenderer.device, internalStateRenderer.uniformBuffersMemory[i]);
    //     vkFreeMemory(internalStateRenderer.device, internalStateRenderer.uniformBuffersMemory[i], NULL);

    //     vkDestroySemaphore(internalStateRenderer.device, internalStateRenderer.imageAvailableSemaphores[i], NULL);
    //     vkDestroyFence(internalStateRenderer.device, internalStateRenderer.inFlightFences[i], NULL);
    // }
    // darray_destroy(internalStateRenderer.uniformBuffers);
    // darray_destroy(internalStateRenderer.uniformBuffersMemory);
    // darray_destroy(internalStateRenderer.uniformBuffersMapped);
    // darray_destroy(internalStateRenderer.imageAvailableSemaphores);
    // darray_destroy(internalStateRenderer.inFlightFences);
    // darray_destroy(internalStateRenderer.renderFinishedSemaphores);

    // vkDestroyBuffer(internalStateRenderer.device, internalStateRenderer.vertexBuffer, NULL);
    // vkFreeMemory(internalStateRenderer.device, internalStateRenderer.vertexBufferMemory, NULL);
    // vkDestroyBuffer(internalStateRenderer.device, internalStateRenderer.indexBuffer, NULL);
    // vkFreeMemory(internalStateRenderer.device, internalStateRenderer.indexBufferMemory, NULL);

    // vkDestroyImage(internalStateRenderer.device, internalStateRenderer.colorImage, NULL);
    // vkDestroyImageView(internalStateRenderer.device, internalStateRenderer.colorImageView, NULL);
    // vkFreeMemory(internalStateRenderer.device, internalStateRenderer.colorImageMemory, NULL);
    // vkDestroyImage(internalStateRenderer.device, internalStateRenderer.depthImage, NULL);
    // vkDestroyImageView(internalStateRenderer.device, internalStateRenderer.depthImageView, NULL);
    // vkFreeMemory(internalStateRenderer.device, internalStateRenderer.depthImageMemory, NULL);

    // vkDestroyCommandPool(internalStateRenderer.device, internalStateRenderer.commandPool, NULL);
    // darray_destroy(internalStateRenderer.commandBuffers);

    // vkDestroyDescriptorPool(internalStateRenderer.device, internalStateRenderer.descriptorPool, NULL);
    // darray_destroy(internalStateRenderer.descriptorSets);

    // vkDestroyPipeline(internalStateRenderer.device, internalStateRenderer.pipeline, NULL);
    // vkDestroyPipelineLayout(internalStateRenderer.device, internalStateRenderer.pipelineLayout, NULL);
    // vkDestroyDescriptorSetLayout(internalStateRenderer.device, internalStateRenderer.descriptorSetLayout, NULL);
    // vkDestroyRenderPass(internalStateRenderer.device, internalStateRenderer.renderPass, NULL);
    // for (uint32_t i = 0; i < darray_get_length(internalStateRenderer.swapchainImages); i++) {
    //     vkDestroyImageView(internalStateRenderer.device, internalStateRenderer.swapchainImageViews[i], NULL);
    //     vkDestroyFramebuffer(internalStateRenderer.device, internalStateRenderer.framebuffers[i], NULL);
    // }
    // darray_destroy(internalStateRenderer.swapchainImages);
    // darray_destroy(internalStateRenderer.swapchainImageViews);
    // darray_destroy(internalStateRenderer.framebuffers);
    // vkDestroySwapchainKHR(internalStateRenderer.device, internalStateRenderer.swapchain, NULL);
    // darray_destroy(internalStateRenderer.swapchainSupportDetails.formats);
    // darray_destroy(internalStateRenderer.swapchainSupportDetails.presentModes);
    // vkDestroyDevice(internalStateRenderer.device, NULL);
    // vkDestroySurfaceKHR(internalStateRenderer.instance, internalStateRenderer.surface, NULL);
    // vkDestroyInstance(internalStateRenderer.instance, NULL);

    // memfree(internalStateVulkan, sizeof(VkState), MEMORY_TAG_RENDERER);
}

void* vulkanRendererThreadEnter(void* arg) {
    vulkanRendererInitialize();
    internalStateRenderer.rendererReady = true;
    mainLoop();
    vulkanCleanUp();
    return 0;
}

void rendererInitialize() {
    uint64_t thread = platformThreadCreate(vulkanRendererThreadEnter, NULL);
    (void)thread;
    while (internalStateRenderer.rendererReady == 0);
}