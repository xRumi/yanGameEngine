#include "renderer_vulkan.h"

VkState* internalStateVulkan;
const char* instanceLayers[] = {"VK_LAYER_KHRONOS_validation"};
const char* instanceExtensions[] = {VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME};
const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME};

extern int platformWindowClosed;

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
    // instace layers
    createInfo.enabledLayerCount = sizeof(instanceLayers) / sizeof(instanceLayers[0]);
    createInfo.ppEnabledLayerNames = instanceLayers;
    // instance extensions
    createInfo.enabledExtensionCount = sizeof(instanceExtensions) / sizeof(instanceExtensions[0]);
    createInfo.ppEnabledExtensionNames = instanceExtensions;
    
    if (vkCreateInstance(&createInfo, NULL, &internalStateVulkan->instance) != VK_SUCCESS) {
        FATAL("Failed to create vk instance");
    }
    
    // // Output vulkan instance extensions to TRACE log
    // uint32_t extCount = 0;
    // vkEnumerateInstanceExtensionProperties(NULL, &extCount, NULL);
    // VkExtensionProperties* extProperties = darray_create_reserve(VkExtensionProperties, extCount);
    // vkEnumerateInstanceExtensionProperties(NULL, &extCount, extProperties);
    // VkExtensionProperties* extNames = darray_create(char);
    // for (int i = 0; i < extCount; i++) {
    //     const char* extName = darray_at_type(extProperties, i, VkExtensionProperties).extensionName;
    //     for (int j = 0; extName[j]; j++) darray_push(extNames, extName[j]);
    //     darray_push(extNames, ',');
    //     darray_push(extNames, ' ');
    // }
    // darray_push(extNames, '\0');
    // TRACE("Vulkan Instance Extensions: %s", darray_at(extNames, 0));
    // darray_destroy(extProperties);
    // darray_destroy(extNames);
}

void createSurface() {
#ifdef __linux__
    VkWaylandSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.surface = platformGetPlatformState()->surface;
    createInfo.display = platformGetPlatformState()->display;
    
    vkCreateWaylandSurfaceKHR(internalStateVulkan->instance, &createInfo, NULL, &internalStateVulkan->surface);
    TRACE("Created wayland vulkan surface");
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, internalStateVulkan->surface, &presentFamily);
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
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, internalStateVulkan->surface, &details.capabilities);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, internalStateVulkan->surface, &formatCount, NULL);
    details.formats = darray_create_reserve(VkSurfaceFormatKHR, formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, internalStateVulkan->surface, &formatCount, details.formats);
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, internalStateVulkan->surface, &presentModeCount, NULL);
    details.presentModes = darray_create_reserve(VkSurfacePresentModeKHR, presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, internalStateVulkan->surface, &presentModeCount, details.presentModes);

    if (formatCount > 0 && presentModeCount > 0) details.isComplete = true;

    return details;
}

VkSampleCountFlagBits getMaxUsableSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(internalStateVulkan->physicalDevice, &physicalDeviceProperties);

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
    vkEnumeratePhysicalDevices(internalStateVulkan->instance, &physicalDeviceCount, NULL);
    VkPhysicalDevice* physicalDevices = darray_create_reserve(VkPhysicalDevice, physicalDeviceCount);
    vkEnumeratePhysicalDevices(internalStateVulkan->instance, &physicalDeviceCount, physicalDevices);

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
        internalStateVulkan->physicalDevice = pickedDevice;
        internalStateVulkan->queueFamilyIndices = pickedQueueFamily;
        internalStateVulkan->swapchainSupportDetails = pickedSwapchainDetails;
        internalStateVulkan->msaaSamples = getMaxUsableSampleCount();
    } else {
        FATAL("Failed to select Physical Device");
    }
}

void createLogicalDevice() {
    const float queuePriority = 1.0f;

    // TODO: implement this with set or hashmap; maybe not??
    VkDeviceQueueCreateInfo* queueCreateInfos = darray_create(VkDeviceQueueCreateInfo);

    VkDeviceQueueCreateInfo graphicsFamilyQueue = {};
    graphicsFamilyQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicsFamilyQueue.queueCount = 1;
    graphicsFamilyQueue.queueFamilyIndex = internalStateVulkan->queueFamilyIndices.graphicsFamily;
    graphicsFamilyQueue.pQueuePriorities = &queuePriority;

    darray_push(queueCreateInfos, graphicsFamilyQueue);

    if (internalStateVulkan->queueFamilyIndices.graphicsFamily != internalStateVulkan->queueFamilyIndices.presentFamily) {
        VkDeviceQueueCreateInfo presentFamilyQueue = {};
        presentFamilyQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentFamilyQueue.queueCount = 1;
        presentFamilyQueue.queueFamilyIndex = internalStateVulkan->queueFamilyIndices.presentFamily;
        presentFamilyQueue.pQueuePriorities = &queuePriority;
        darray_push(queueCreateInfos, presentFamilyQueue);
    }

    VkPhysicalDeviceFeatures features = {};
    features.samplerAnisotropy = true;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = darray_get_length(queueCreateInfos);
    createInfo.ppEnabledLayerNames = instanceLayers;
    createInfo.enabledLayerCount = sizeof(instanceLayers) / sizeof(instanceLayers[0]);
    createInfo.ppEnabledExtensionNames = deviceExtensions;
    createInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
    createInfo.pEnabledFeatures = &features;
    if (vkCreateDevice(internalStateVulkan->physicalDevice, &createInfo, NULL, &internalStateVulkan->device) != VK_SUCCESS) {
        FATAL("Failed to create vulkan device");
    }
    darray_destroy(queueCreateInfos);
    vkGetDeviceQueue(internalStateVulkan->device, internalStateVulkan->queueFamilyIndices.graphicsFamily, 0, &internalStateVulkan->graphicsQueue);
    vkGetDeviceQueue(internalStateVulkan->device, internalStateVulkan->queueFamilyIndices.presentFamily, 0, &internalStateVulkan->presentQueue);
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(internalStateVulkan->physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    FATAL("failed to find suitable memory type");
}

void createImage(uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory) {
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
    
    if (vkCreateImage(internalStateVulkan->device, &imageInfo, NULL, image) != VK_SUCCESS) {
        FATAL("Failed to create vulkan image");
    }

    VkMemoryRequirements memRequirements = {};
    vkGetImageMemoryRequirements(internalStateVulkan->device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(internalStateVulkan->device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
        FATAL("Failed to allocate texture image memory");
    }
    vkBindImageMemory(internalStateVulkan->device, *image, *imageMemory, 0);
}

void createImageView(VkImageView* imageView, VkImage image, VkFormat imageFormat, uint32_t mipLevels, VkImageAspectFlags imageAspect) {
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

    if (vkCreateImageView(internalStateVulkan->device, &createInfo, NULL, imageView) != VK_SUCCESS) {
        FATAL("Failed to create image view");
    }
}

void createSwapchain() {
    // Select format and presentMode from available formats and present modes
    VkSurfaceFormatKHR format = internalStateVulkan->swapchainSupportDetails.formats[0];
    for (uint32_t i = 0; i < darray_get_length(internalStateVulkan->swapchainSupportDetails.formats); i++) {
        if (darray_at_type(internalStateVulkan->swapchainSupportDetails.formats, i, VkSurfaceFormatKHR).format == VK_FORMAT_B8G8R8A8_SRGB && darray_at_type(internalStateVulkan->swapchainSupportDetails.formats, i, VkSurfaceFormatKHR).colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            format = darray_at_type(internalStateVulkan->swapchainSupportDetails.formats, i, VkSurfaceFormatKHR);
            break;
        }
    }
    // Every device that have present queue got support for FIFO present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < darray_get_length(internalStateVulkan->swapchainSupportDetails.presentModes); i++) {
        if (darray_at_type(internalStateVulkan->swapchainSupportDetails.presentModes, i, VkPresentModeKHR) == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    VkExtent2D extent = internalStateVulkan->swapchainSupportDetails.capabilities.currentExtent;
    extent.height = internalStateVulkan->height;
    extent.width = internalStateVulkan->width;

    uint32_t imageCount = internalStateVulkan->swapchainSupportDetails.capabilities.minImageCount + 1;
    if (internalStateVulkan->swapchainSupportDetails.capabilities.maxImageCount != 0 && imageCount > internalStateVulkan->swapchainSupportDetails.capabilities.maxImageCount) {
        imageCount = internalStateVulkan->swapchainSupportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = internalStateVulkan->surface;
    createInfo.presentMode = presentMode;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.minImageCount = imageCount;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = internalStateVulkan->swapchainSupportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (internalStateVulkan->queueFamilyIndices.graphicsFamily == internalStateVulkan->queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    } else {
        uint32_t indices[] = {internalStateVulkan->queueFamilyIndices.graphicsFamily, internalStateVulkan->queueFamilyIndices.presentFamily};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }

    if (vkCreateSwapchainKHR(internalStateVulkan->device, &createInfo, NULL, &internalStateVulkan->swapchain) != VK_SUCCESS) {
        FATAL("Failed to create swapchain");
    }
    TRACE("Swapchain created");

    // get swapchain images and create image views for them
    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(internalStateVulkan->device, internalStateVulkan->swapchain, &swapchainImageCount, NULL);
    internalStateVulkan->swapchainImages = darray_create_reserve(VkImage, swapchainImageCount);
    vkGetSwapchainImagesKHR(internalStateVulkan->device, internalStateVulkan->swapchain, &swapchainImageCount, internalStateVulkan->swapchainImages);
    TRACE("Swapchain image count: %d", swapchainImageCount);
    
    internalStateVulkan->swapchainImageFormat = format.format;
    internalStateVulkan->swapchainImageExtent = extent;
    internalStateVulkan->swapchainImageViews = darray_create_reserve(VkImageView, swapchainImageCount);
    for (uint32_t i = 0; i < swapchainImageCount; i++)
        createImageView(&internalStateVulkan->swapchainImageViews[i], internalStateVulkan->swapchainImages[i], internalStateVulkan->swapchainImageFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkFormat findSupportedFormat(VkFormat* candidates, uint32_t formatCount, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (int32_t i = 0; i < formatCount; i++) {
        VkFormat format = candidates[i];
        VkFormatProperties properties = {};
        vkGetPhysicalDeviceFormatProperties(internalStateVulkan->physicalDevice, format, &properties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) return format;
    }
    FATAL("Failed to find supported format");
}

void createRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = internalStateVulkan->swapchainImageFormat;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.samples = internalStateVulkan->msaaSamples;

    VkFormat formatChoice[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    internalStateVulkan->depthFormat = findSupportedFormat(formatChoice, sizeof(formatChoice) / sizeof(formatChoice[0]), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = internalStateVulkan->depthFormat;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.samples = internalStateVulkan->msaaSamples;

    VkAttachmentDescription colorAttachmentResolve = {};
    colorAttachmentResolve.format = internalStateVulkan->swapchainImageFormat;
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

    if (vkCreateRenderPass(internalStateVulkan->device, &createInfo, NULL, &internalStateVulkan->renderPass) != VK_SUCCESS) {
        FATAL("Failed to create renderPass");
    }
}

void createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = sizeof(bindings) / sizeof(bindings[0]);
    createInfo.pBindings = bindings;
    if (vkCreateDescriptorSetLayout(internalStateVulkan->device, &createInfo, NULL, &internalStateVulkan->descriptorSetLayout) != VK_SUCCESS) {
        FATAL("Failed to create descriptor set layout");
    }
}

VkShaderModule createShaderModule(const char* shaderCode) {
    VkShaderModule shaderModule = {};
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = darray_get_length(shaderCode);
    createInfo.pCode = (const uint32_t*)shaderCode;
    if (vkCreateShaderModule(internalStateVulkan->device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        FATAL("Failed to create shader module");
    }
    return shaderModule;
}

VkVertexInputBindingDescription getBindingdescription() { // TODO: move to suitable location
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}
VkVertexInputAttributeDescription* getAttributeDescriptions() {
    VkVertexInputAttributeDescription* attributeDescriptions = darray_create_reserve(VkVertexInputAttributeDescription, 3);
    
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

void createGraphicsPipline() {
    char* vertShaderCode = readFile("./src/assets/shaders/vert.spv");
    char* fragShaderCode = readFile("./src/assets/shaders/frag.spv");
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
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

    VkVertexInputBindingDescription bindingDescription = getBindingdescription();
    VkVertexInputAttributeDescription* attributeDescriptions = getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexCreateInfo = {};
    vertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexCreateInfo.vertexBindingDescriptionCount = 1;
    vertexCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexCreateInfo.vertexAttributeDescriptionCount = darray_get_length(attributeDescriptions);
    vertexCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = (float)internalStateVulkan->height;
    viewport.width = (float)internalStateVulkan->width;
    viewport.height = -(float)internalStateVulkan->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = internalStateVulkan->swapchainImageExtent;

    VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
    viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportCreateInfo.pViewports = &viewport;
    viewportCreateInfo.viewportCount = 1;
    viewportCreateInfo.pScissors = &scissor;
    viewportCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationCreateInfo.lineWidth = 1.0f;
    rasterizationCreateInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = internalStateVulkan->msaaSamples;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &internalStateVulkan->descriptorSetLayout;

    if (vkCreatePipelineLayout(internalStateVulkan->device, &pipelineLayoutCreateInfo, NULL, &internalStateVulkan->pipelineLayout) != VK_SUCCESS) {
        FATAL("Failed to create pipeline layout!");
    }
    
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;
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
    createInfo.layout = internalStateVulkan->pipelineLayout;
    createInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    createInfo.pVertexInputState = &vertexCreateInfo;
    createInfo.pRasterizationState = &rasterizationCreateInfo;
    createInfo.pColorBlendState = &colorBlendCreateInfo;
    createInfo.pDynamicState = &dynamicCreateInfo;
    createInfo.pMultisampleState = &multisampleCreateInfo;
    createInfo.pViewportState = &viewportCreateInfo;
    createInfo.renderPass = internalStateVulkan->renderPass;
    createInfo.pDepthStencilState = &depthStencilCreateInfo;
    createInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(internalStateVulkan->device, NULL, 1, &createInfo, NULL, &internalStateVulkan->pipeline) != VK_SUCCESS) {
        FATAL("Failed to create graphics pipeline");
    }
    TRACE("Graphics pipeline created");

    darray_destroy(attributeDescriptions);
    vkDestroyShaderModule(internalStateVulkan->device, vertShaderModule, NULL);
    vkDestroyShaderModule(internalStateVulkan->device, fragShaderModule, NULL);
}

void createCommandPool() {
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = internalStateVulkan->queueFamilyIndices.graphicsFamily;
    
    if (vkCreateCommandPool(internalStateVulkan->device, &createInfo, NULL, &internalStateVulkan->commandPool) != VK_SUCCESS) {
        FATAL("Failed to create command pool");
    }
}

void createColorResources() {
    createImage(internalStateVulkan->swapchainImageExtent.width, internalStateVulkan->swapchainImageExtent.height, internalStateVulkan->swapchainImageFormat, 1, internalStateVulkan->msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &internalStateVulkan->colorImage, &internalStateVulkan->colorImageMemory);
    createImageView(&internalStateVulkan->colorImageView, internalStateVulkan->colorImage, internalStateVulkan->swapchainImageFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = internalStateVulkan->commandPool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    if (vkAllocateCommandBuffers(internalStateVulkan->device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        FATAL("Failed to allocate single time command buffer");
    }
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(internalStateVulkan->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkDeviceWaitIdle(internalStateVulkan->device);
    vkFreeCommandBuffers(internalStateVulkan->device, internalStateVulkan->commandPool, 1, &commandBuffer);
}
void transitionImageLayout(VkImage image, VkFormat format, uint32_t mipLevels,VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
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
    endSingleTimeCommands(commandBuffer);
}

void createDepthResources() {
    VkFormat depthFormat = internalStateVulkan->depthFormat;
    createImage(internalStateVulkan->swapchainImageExtent.width, internalStateVulkan->swapchainImageExtent.height, depthFormat, 1, internalStateVulkan->msaaSamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &internalStateVulkan->depthImage, &internalStateVulkan->depthImageMemory);
    createImageView(&internalStateVulkan->depthImageView, internalStateVulkan->depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
    transitionImageLayout(internalStateVulkan->depthImage, depthFormat, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void createFramebuffers() {
    internalStateVulkan->framebuffers = darray_create_reserve(VkFramebuffer, darray_get_length(internalStateVulkan->swapchainImages));
    for (uint32_t i = 0; i < darray_get_length(internalStateVulkan->swapchainImages); i++) {
        VkImageView attachments[] = {internalStateVulkan->colorImageView, internalStateVulkan->depthImageView, internalStateVulkan->swapchainImageViews[i]};
        VkFramebufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = internalStateVulkan->renderPass;
        createInfo.attachmentCount = CARRAY_SIZE(attachments);
        createInfo.pAttachments = attachments;
        createInfo.width = internalStateVulkan->swapchainImageExtent.width;
        createInfo.height = internalStateVulkan->swapchainImageExtent.height;
        createInfo.layers = 1;
        
        if (vkCreateFramebuffer(internalStateVulkan->device, &createInfo, NULL, &internalStateVulkan->framebuffers[i]) != VK_SUCCESS) {
            FATAL("Failed to create framebuffer");
        }
    }
}

// void createTextureImage() {
//     int width = 0, height = 0, channel = 0;
//     stbi_uc* pixels = stbi_load("textures/viking_room.png", &width, &height, &channel, STBI_rgb_alpha);
//     if (!pixels) {
//         throw std::runtime_error("Failed to load texture image");
//     }
//     VkDeviceSize imageSize = width * height * 4;
//     VkBuffer staggingBuffer;
//     VkDeviceMemory staggingMemory;
//     createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staggingBuffer, staggingMemory);
    
//     void* data;
//     vkMapMemory(device, staggingMemory, 0, imageSize, 0, &data);
//     memcpy(data, pixels, imageSize);
//     vkUnmapMemory(device, staggingMemory);

//     stbi_image_free(pixels);

//     mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

//     createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
//     transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//     copyBufferToImage(staggingBuffer, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
//     // transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//     generateMipmaps(textureImage, width, height, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);

//     vkDestroyBuffer(device, staggingBuffer, nullptr);
//     vkFreeMemory(device, staggingMemory, nullptr);

//     createImageView(textureImageView, textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_IMAGE_ASPECT_COLOR_BIT);
// }

void createTextureSampler() {
    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(internalStateVulkan->physicalDevice, &properties);

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

    if (vkCreateSampler(internalStateVulkan->device, &createInfo, NULL, &internalStateVulkan->textureSampler) != VK_SUCCESS) {
        FATAL("Failed to create texture sampler");
    }
}

// void loadModel() {
//     tinyobj::attrib_t attrib;
//     std::vector<tinyobj::shape_t> shapes;
//     std::vector<tinyobj::material_t> materials;
//     std::string warn, err;
//     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "models/viking_room.obj")) {
//         throw std::runtime_error("Tinyobjloader error: " + err);
//     };
//     if (!warn.empty()) std::cerr << "Tinyobjloader warn: " << warn << '\n';
//     std::unordered_map<Vertex, uint32_t> uniqueVertices{};
//     for (const tinyobj::shape_t& shape : shapes) {
//         for (const tinyobj::index_t& index : shape.mesh.indices) {
//             Vertex vertex;
//             vertex.position = {
//                 attrib.vertices[index.vertex_index * 3 + 0],
//                 attrib.vertices[index.vertex_index * 3 + 1],
//                 attrib.vertices[index.vertex_index * 3 + 2]
//             };
//             vertex.texCoord = {
//                 attrib.texcoords[index.texcoord_index * 2 + 0],
//                 1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]
//             };
//             vertex.color = {1.0f, 1.0f, 1.0f};
//             if (uniqueVertices.count(vertex) == 0) {
//                 uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
//                 vertices.push_back(vertex);
//             }
//             indices.push_back(uniqueVertices[vertex]);
//         }
//     }
// }

#include "asset_manager.h"

void loadModel() {

    Model* model = load_model("./assets/world/models/BoxVertexColors/BoxVertexColors.gltf");

    darray_foreach(model->mesh, Mesh, x, {
        if (x.vertices) {
            internalStateVulkan->vertices = x.vertices;
            internalStateVulkan->indices = x.indices;
            break;
        }
    });

    DEBUG("loadded with verties: %d indices: %d", darray_get_length(internalStateVulkan->vertices), darray_get_length(internalStateVulkan->indices));

    // internalStateVulkan->vertices = darray_create_reserve(Vertex, 3);
    // internalStateVulkan->indices = darray_create_reserve(uint32_t, 3);
    // Vertex vertices[3] = {
    //     {
    //         {{-0.5, -0.5, 0}},
    //         {{1, 0, 0}},
    //         {{0, 0}}
    //     },
    //     {
    //         {{0.5, -0.5, 0}},
    //         {{0, 0, 1}},
    //         {{0, 0}}
    //     },
    //     {
    //         {{0, 0.5, 0}},
    //         {{0, 1, 0}},
    //         {{0, 0}}
    //     },
    // };
    // uint32_t indices[3] = {0, 1, 2};
    // memcpy(internalStateVulkan->vertices, vertices, sizeof(vertices));
    // memcpy(internalStateVulkan->indices, indices, sizeof(indices));
    // TRACE("Triangle model loaded");
}

void updateUniformBuffers(double deltaTime) {
    double elapsedTime = platformGetTime() - internalStateVulkan->startTime;
    (void)elapsedTime;

    UniformBufferObject ubo = {};

    // ubo.model = mat4_identity();
    // ubo.model = mat4_rotation_z(elapsedTime * 20);
    // ubo.model = mat4_scale(elapsedTime, elapsedTime, elapsedTime);
    ubo.model = mat4_mul(mat4_rotation_x(45 * elapsedTime), mat4_rotation_y(45 * elapsedTime));
    // ubo.model = mat4_translation(0, elapsedTime * 0.1, 0);

    mat4 proj = mat4_perspective(90, (float)internalStateVulkan->width / (float)internalStateVulkan->height, 0.1f, 100.0f);

    vec3 camera = {{0, 0, 3}};
    vec3 target = {{0.0f, 0.0f, 0.0f}};
    vec3 up     = {{0.0f, 1.0f, 0.0f}};

    mat4 view = mat4_look_at(camera, target, up);

    ubo.view = view;
    ubo.projection = proj;

    memcpy(internalStateVulkan->uniformBuffersMapped[internalStateVulkan->currentFrame], &ubo, sizeof(ubo));
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(internalStateVulkan->device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        FATAL("Failed to create vulkan buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(internalStateVulkan->device, *buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(internalStateVulkan->device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        FATAL("Failed to allocate vulkan buffer memory");
    };
    vkBindBufferMemory(internalStateVulkan->device, *buffer, *bufferMemory, 0);
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void createVertexBuffer() {
    VkDeviceSize bufferSize = darray_get_length(internalStateVulkan->vertices) * sizeof(Vertex);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
    void* data;
    vkMapMemory(internalStateVulkan->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, internalStateVulkan->vertices, bufferSize);
    vkUnmapMemory(internalStateVulkan->device, stagingBufferMemory);
    createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &internalStateVulkan->vertexBuffer, &internalStateVulkan->vertexBufferMemory);
    copyBuffer(stagingBuffer, internalStateVulkan->vertexBuffer, bufferSize);
    vkDestroyBuffer(internalStateVulkan->device, stagingBuffer, NULL);
    vkFreeMemory(internalStateVulkan->device, stagingBufferMemory, NULL);
}

void createIndexBuffer() {
    VkDeviceSize bufferSize = darray_get_length(internalStateVulkan->indices) * sizeof(internalStateVulkan->indices[0]);
    VkBuffer staggingBuffer;
    VkDeviceMemory staggingMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staggingBuffer, &staggingMemory);
    void *data;
    vkMapMemory(internalStateVulkan->device, staggingMemory, 0, bufferSize, 0, &data);
    memcpy(data, internalStateVulkan->indices, bufferSize);
    vkUnmapMemory(internalStateVulkan->device, staggingMemory);
    createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &internalStateVulkan->indexBuffer, &internalStateVulkan->indexBufferMemory);
    copyBuffer(staggingBuffer, internalStateVulkan->indexBuffer, bufferSize);
    vkDestroyBuffer(internalStateVulkan->device, staggingBuffer, NULL);
    vkFreeMemory(internalStateVulkan->device, staggingMemory, NULL);
}

void createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    internalStateVulkan->uniformBuffers = darray_create_reserve(UniformBufferObject, MAX_FRAMES_IN_FLIGHT);
    internalStateVulkan->uniformBuffersMemory = darray_create_reserve(VkDeviceMemory, MAX_FRAMES_IN_FLIGHT);
    internalStateVulkan->uniformBuffersMapped = darray_create_reserve(void*, MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &internalStateVulkan->uniformBuffers[i], &internalStateVulkan->uniformBuffersMemory[i]);
        vkMapMemory(internalStateVulkan->device, internalStateVulkan->uniformBuffersMemory[i], 0, bufferSize, 0, &internalStateVulkan->uniformBuffersMapped[i]);
    }
}

void createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = CARRAY_SIZE(poolSizes);
    createInfo.pPoolSizes = poolSizes;
    createInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
    if (vkCreateDescriptorPool(internalStateVulkan->device, &createInfo, NULL, &internalStateVulkan->descriptorPool) != VK_SUCCESS) {
        FATAL("Failed to create descriptor pool");
    }
}

void createDescriptorSet() {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {internalStateVulkan->descriptorSetLayout, internalStateVulkan->descriptorSetLayout};
    internalStateVulkan->descriptorSets = darray_create_reserve(VkDescriptorSet, MAX_FRAMES_IN_FLIGHT);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = internalStateVulkan->descriptorPool;
    allocInfo.pSetLayouts = layouts;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    if (vkAllocateDescriptorSets(internalStateVulkan->device, &allocInfo, internalStateVulkan->descriptorSets) != VK_SUCCESS) {
        FATAL("Failed to allocate descriptor sets");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = internalStateVulkan->uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = internalStateVulkan->textureImageView;
        imageInfo.sampler = internalStateVulkan->textureSampler;

        VkWriteDescriptorSet writeDescriptors[2] = {};
        writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptors[0].descriptorCount = 1;
        writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptors[0].dstSet = internalStateVulkan->descriptorSets[i];
        writeDescriptors[0].pBufferInfo = &bufferInfo;
        writeDescriptors[0].dstBinding = 0;
        writeDescriptors[0].dstArrayElement = 0;

        writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptors[1].descriptorCount = 1;
        writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptors[1].dstSet = internalStateVulkan->descriptorSets[i];
        writeDescriptors[1].pImageInfo = &imageInfo;
        writeDescriptors[1].dstBinding = 1;
        writeDescriptors[1].dstArrayElement = 0;

        // TODO: add image smpler
        vkUpdateDescriptorSets(internalStateVulkan->device, 1, writeDescriptors, 0, NULL);
    }
}

void createCommandBuffer() {
    internalStateVulkan->commandBuffers = darray_create_reserve(VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.commandPool = internalStateVulkan->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    if (vkAllocateCommandBuffers(internalStateVulkan->device, &allocInfo, internalStateVulkan->commandBuffers) != VK_SUCCESS) {
        FATAL("Failed to allocate command buffer");
    }
}

void createSyncObjects() {
    internalStateVulkan->imageAvailableSemaphores = darray_create_reserve(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    internalStateVulkan->renderFinishedSemaphores = darray_create_reserve(VkSemaphore, darray_get_length(internalStateVulkan->swapchainImageViews));
    internalStateVulkan->inFlightFences = darray_create_reserve(VkFence, MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(internalStateVulkan->device, &semaphoreCreateInfo, NULL, &internalStateVulkan->imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(internalStateVulkan->device, &fenceCreateInfo, NULL, &internalStateVulkan->inFlightFences[i]) != VK_SUCCESS) {
            FATAL("Failed to create sync objects");
        }
    }

    for (int i = 0; i < darray_get_length(internalStateVulkan->swapchainImageViews); i++) {
        if (vkCreateSemaphore(internalStateVulkan->device, &semaphoreCreateInfo, NULL, &internalStateVulkan->renderFinishedSemaphores[i])) {
            FATAL("Failed to create sync objects");
        }
    }
}

void vulkanInitialization() {
    internalStateVulkan = memalloc(sizeof(VkState), MEMORY_TAG_RENDERER);
    memset(internalStateVulkan, 0, sizeof(VkState));
    internalStateVulkan->width = platformGetPlatformState()->width;
    internalStateVulkan->height = platformGetPlatformState()->height;
    internalStateVulkan->startTime = platformGetTime();

    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipline();
    createCommandPool();
    createColorResources();
    createDepthResources();
    createFramebuffers();
    // createTextureImage();
    // createTextureSampler();
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSet();
    createCommandBuffer();
    createSyncObjects();
}

void recordCommandBuffer(const VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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
    renderPassInfo.renderPass = internalStateVulkan->renderPass;
    renderPassInfo.framebuffer = internalStateVulkan->framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = internalStateVulkan->swapchainImageExtent;
    renderPassInfo.clearValueCount = CARRAY_SIZE(clearColors);
    renderPassInfo.pClearValues = clearColors;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, internalStateVulkan->pipeline);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = (float)internalStateVulkan->height;
    viewport.width = (float)internalStateVulkan->width;
    viewport.height = -(float)internalStateVulkan->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = internalStateVulkan->swapchainImageExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &internalStateVulkan->vertexBuffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, internalStateVulkan->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, internalStateVulkan->pipelineLayout, 0, 1, &internalStateVulkan->descriptorSets[internalStateVulkan->currentFrame], 0, NULL);
    
    vkCmdDrawIndexed(commandBuffer, darray_get_length(internalStateVulkan->indices), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        FATAL("Failed to record command buffer");
    }
}

void drawFrame(double deltaTime) {
    vkWaitForFences(internalStateVulkan->device, 1, &internalStateVulkan->inFlightFences[internalStateVulkan->currentFrame], VK_TRUE, UINT32_MAX);
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(internalStateVulkan->device, internalStateVulkan->swapchain, UINT32_MAX, internalStateVulkan->imageAvailableSemaphores[internalStateVulkan->currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // recreateSwapchain();
        WARN("Need to recreate swapchain but not implemented");
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        FATAL("Failed to acquire next swapchain image");
    }
    vkResetFences(internalStateVulkan->device, 1, &internalStateVulkan->inFlightFences[internalStateVulkan->currentFrame]);
    vkResetCommandBuffer(internalStateVulkan->commandBuffers[internalStateVulkan->currentFrame], 0);
    recordCommandBuffer(internalStateVulkan->commandBuffers[internalStateVulkan->currentFrame], imageIndex);
    VkPipelineStageFlags waitStage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &internalStateVulkan->commandBuffers[internalStateVulkan->currentFrame];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &internalStateVulkan->imageAvailableSemaphores[internalStateVulkan->currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &internalStateVulkan->renderFinishedSemaphores[imageIndex];
    submitInfo.pWaitDstStageMask = waitStage;

    updateUniformBuffers(deltaTime);
    
    if (vkQueueSubmit(internalStateVulkan->graphicsQueue, 1, &submitInfo, internalStateVulkan->inFlightFences[internalStateVulkan->currentFrame]) != VK_SUCCESS) {
        FATAL("Failed to submit draw command to queue");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &internalStateVulkan->renderFinishedSemaphores[imageIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &internalStateVulkan->swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    result = vkQueuePresentKHR(internalStateVulkan->presentQueue, &presentInfo);
    if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || internalStateVulkan->framebufferResized) {
        internalStateVulkan->framebufferResized = false;
        // recreateSwapchain();
        WARN("Need to recreate swapchain but not implemented");
        return;
    } else if (result != VK_SUCCESS) {
        FATAL("Failed to present swapchain image");
    }
    internalStateVulkan->currentFrame = (internalStateVulkan->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void mainLoop() {
    double fpsCap = 144;
    double targetFrameTime = 1 / fpsCap;
    double deltaTime = 0;

    double currentTime, lastTime = platformGetTime();
    while (!platformWindowClosed) {
        currentTime = platformGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        drawFrame(deltaTime);
    
        double frameTime = platformGetTime() - currentTime;
        if (frameTime < targetFrameTime) {
            double sleepTime = targetFrameTime - frameTime;
            platformSleep(sleepTime);
        }
    }
}

void vulkanCleanUp() {
    vkDeviceWaitIdle(internalStateVulkan->device);

    darray_destroy(internalStateVulkan->vertices);
    darray_destroy(internalStateVulkan->indices);

    for (int i = 0; i < darray_get_length(internalStateVulkan->swapchainImages); i++) {
        vkDestroySemaphore(internalStateVulkan->device, internalStateVulkan->renderFinishedSemaphores[i], NULL);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(internalStateVulkan->device, internalStateVulkan->uniformBuffers[i], NULL);
        vkUnmapMemory(internalStateVulkan->device, internalStateVulkan->uniformBuffersMemory[i]);
        vkFreeMemory(internalStateVulkan->device, internalStateVulkan->uniformBuffersMemory[i], NULL);

        vkDestroySemaphore(internalStateVulkan->device, internalStateVulkan->imageAvailableSemaphores[i], NULL);
        vkDestroyFence(internalStateVulkan->device, internalStateVulkan->inFlightFences[i], NULL);
    }
    darray_destroy(internalStateVulkan->uniformBuffers);
    darray_destroy(internalStateVulkan->uniformBuffersMemory);
    darray_destroy(internalStateVulkan->uniformBuffersMapped);
    darray_destroy(internalStateVulkan->imageAvailableSemaphores);
    darray_destroy(internalStateVulkan->inFlightFences);
    darray_destroy(internalStateVulkan->renderFinishedSemaphores);

    vkDestroyBuffer(internalStateVulkan->device, internalStateVulkan->vertexBuffer, NULL);
    vkFreeMemory(internalStateVulkan->device, internalStateVulkan->vertexBufferMemory, NULL);
    vkDestroyBuffer(internalStateVulkan->device, internalStateVulkan->indexBuffer, NULL);
    vkFreeMemory(internalStateVulkan->device, internalStateVulkan->indexBufferMemory, NULL);

    vkDestroyImage(internalStateVulkan->device, internalStateVulkan->colorImage, NULL);
    vkDestroyImageView(internalStateVulkan->device, internalStateVulkan->colorImageView, NULL);
    vkFreeMemory(internalStateVulkan->device, internalStateVulkan->colorImageMemory, NULL);
    vkDestroyImage(internalStateVulkan->device, internalStateVulkan->depthImage, NULL);
    vkDestroyImageView(internalStateVulkan->device, internalStateVulkan->depthImageView, NULL);
    vkFreeMemory(internalStateVulkan->device, internalStateVulkan->depthImageMemory, NULL);

    vkDestroyCommandPool(internalStateVulkan->device, internalStateVulkan->commandPool, NULL);
    darray_destroy(internalStateVulkan->commandBuffers);

    vkDestroyDescriptorPool(internalStateVulkan->device, internalStateVulkan->descriptorPool, NULL);
    darray_destroy(internalStateVulkan->descriptorSets);

    vkDestroyPipeline(internalStateVulkan->device, internalStateVulkan->pipeline, NULL);
    vkDestroyPipelineLayout(internalStateVulkan->device, internalStateVulkan->pipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(internalStateVulkan->device, internalStateVulkan->descriptorSetLayout, NULL);
    vkDestroyRenderPass(internalStateVulkan->device, internalStateVulkan->renderPass, NULL);
    for (uint32_t i = 0; i < darray_get_length(internalStateVulkan->swapchainImages); i++) {
        vkDestroyImageView(internalStateVulkan->device, internalStateVulkan->swapchainImageViews[i], NULL);
        vkDestroyFramebuffer(internalStateVulkan->device, internalStateVulkan->framebuffers[i], NULL);
    }
    darray_destroy(internalStateVulkan->swapchainImages);
    darray_destroy(internalStateVulkan->swapchainImageViews);
    darray_destroy(internalStateVulkan->framebuffers);
    vkDestroySwapchainKHR(internalStateVulkan->device, internalStateVulkan->swapchain, NULL);
    darray_destroy(internalStateVulkan->swapchainSupportDetails.formats);
    darray_destroy(internalStateVulkan->swapchainSupportDetails.presentModes);
    vkDestroyDevice(internalStateVulkan->device, NULL);
    vkDestroySurfaceKHR(internalStateVulkan->instance, internalStateVulkan->surface, NULL);
    vkDestroyInstance(internalStateVulkan->instance, NULL);

    memfree(internalStateVulkan, sizeof(VkState), MEMORY_TAG_RENDERER);
}

void* vulkanRendererThreadEnter(void* arg) {
    vulkanInitialization();
    mainLoop();
    vulkanCleanUp();
    return 0;
}

void rendererInitialize() {
    uint64_t thread = platformThreadCreate(vulkanRendererThreadEnter, NULL);
    (void)thread;
}