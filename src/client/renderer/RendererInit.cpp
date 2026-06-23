// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"
#include "../../shared/version.hpp"
#include "../../shared/local.hpp"

#include <fstream>

namespace VE
{
    Renderer::Renderer(GLFWwindow *window, Size2 windowSize)
    {
        this->window = window;

        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();

        findSupportedFormats();

        createCommandPool();
        createCommandBuffers();

        createSwapChain(windowSize);

        createPrePostImages();

        createPipelineCache();

        createTextureSampler();
        createTextureDescriptorSetLayout();
        createFallbackTexture();

        createShadowDepthAttachment();
        createShadowPipeline();

        createDepthAttachments();
        createShadowSampler();
        createModelUniformBuffers();
        createModelDescriptors();
        createModelPipeline();

        createTransparentPipeline();

        createPostSampler();
        createPostDescriptors();
        createPostPipeline();

        createUIUniformBuffers();
        createUIDescriptors();
        createUIPipeline();

        createSyncObjects();

        Log::add('R', 000);
    }

    void Renderer::createInstance()
    {
        VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Verge Engine",
            .applicationVersion = VK_MAKE_VERSION(0, 0, VERGE_ENGINE_VERSION),
            .apiVersion = VK_API_VERSION_1_3};

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (DEVELOPER_MODE)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

        VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = 1,
            .ppEnabledLayerNames = validationLayers,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()};

        if (DEVELOPER_MODE)
        {
            instanceCreateInfo.enabledLayerCount = 1;
            instanceCreateInfo.ppEnabledLayerNames = validationLayers;
        }
        else
        {
            instanceCreateInfo.enabledLayerCount = 0;
            instanceCreateInfo.ppEnabledLayerNames = nullptr;
        }

        vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), {'R', 200});
    }

    void Renderer::createSurface()
    {
        vkCheck(glfwCreateWindowSurface(instance, window, nullptr, &surface), {'R', 201});
    }

    void Renderer::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr), {'R', 202});
        if (deviceCount == 0)
            Log::add('R', 202);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()), {'R', 202});

        int bestScore = 0;
        for (VkPhysicalDevice device : devices)
        {
            int score = rateDevice(device, surface);
            if (score > bestScore)
            {
                bestScore = score;
                physicalDevice = device;
            }
        }

        if (bestScore == 0)
            Log::add('R', 202);
    }

    void Renderer::createLogicalDevice()
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const VkQueueFamilyProperties &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueueFamilyIndex = i;
                break;
            }
            i++;
        }

        transferQueueFamilyIndex = graphicsQueueFamilyIndex;
        int j = 0;
        for (const VkQueueFamilyProperties &queueFamily : queueFamilies)
        {
            bool hasTransfer = queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT;
            bool hasGraphics = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            if (hasTransfer && !hasGraphics && queueFamily.queueCount > 0)
            {
                transferQueueFamilyIndex = j;
                break;
            }
            j++;
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphicsQueueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority};

        queueCreateInfos.push_back(graphicsQueueCreateInfo);

        if (transferQueueFamilyIndex != graphicsQueueFamilyIndex)
        {
            VkDeviceQueueCreateInfo transferQueueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = transferQueueFamilyIndex,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority};
            queueCreateInfos.push_back(transferQueueCreateInfo);
        }

        const char *swapChainExtention = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        VkPhysicalDeviceFeatures deviceFeatures = {
            .samplerAnisotropy = VK_TRUE};

        VkPhysicalDeviceSynchronization2Features sync2Features{};
        sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        sync2Features.synchronization2 = VK_TRUE;

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = chainStructs(&dynamicRenderingFeatures, &sync2Features),
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = 1,
            .ppEnabledExtensionNames = &swapChainExtention,
            .pEnabledFeatures = &deviceFeatures};

        vkCheck(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device), {'R', 203});

        vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(device, transferQueueFamilyIndex, 0, &transferQueue);
        presentQueue = graphicsQueue;
    }

    void Renderer::findSupportedFormats()
    {
        // Color format
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, availableFormats.data());

        for (const VkSurfaceFormatKHR &format : availableFormats)
        {
            if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                swapChain.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
                break;
            }
        }

        if(swapChain.imageFormat == VK_FORMAT_UNDEFINED)
            Log::add('R', 225);

        // Depth format
        std::vector<VkFormat> depthFormats = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

        for (VkFormat f : depthFormats)
        {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, f, &properties);

            if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                depthFormat = f;
                break;
            }
        }

        if (depthFormat == VK_FORMAT_UNDEFINED)
            Log::add('R', 223);

        // Present mode
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, availablePresentModes.data());

        for (VkPresentModeKHR presentMode : availablePresentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapChain.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }
    }

    void Renderer::createSwapChain(Size2 windowSize)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        swapChain.extent = {windowSize.w, windowSize.h};

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = capabilities.minImageCount + (capabilities.minImageCount != capabilities.maxImageCount),
            .imageFormat = swapChain.imageFormat,
            .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = swapChain.extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = swapChain.presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE};

        vkCheck(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain.swapChain), {'R', 204});

        vkGetSwapchainImagesKHR(device, swapChain, &swapChain.imageCount, nullptr);
        swapChain.images.resize(swapChain.imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &swapChain.imageCount, swapChain.images.data());

        swapChain.imageViews.resize(swapChain.imageCount);
        for (size_t i = 0; i < swapChain.imageCount; i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = swapChain.images[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = swapChain.imageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapChain.imageViews[i]), {'R', 205});
        }
    }

    void Renderer::createCommandPool()
    {
        VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = graphicsQueueFamilyIndex};

        vkCheck(vkCreateCommandPool(device, &graphicsCommandPoolCreateInfo, nullptr, &commandPool), {'R', 208});
    }

    void Renderer::createCommandBuffers()
    {
        std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> commandBuffers;

        VkCommandBufferAllocateInfo commandBufferAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())};

        vkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, commandBuffers.data()), {'R', 212});

        for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            frames[i].commandBuffer = commandBuffers[i];
    }

    void Renderer::createPrePostImages()
    {
        prePostAttachments.resize(swapChain.imageCount);
        for (ImageAttachment &attachment : prePostAttachments)
        {
            attachment.image = createImage(swapChain.extent.width, swapChain.extent.height, swapChain.imageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, &attachment.memory);

            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = attachment.image;
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = swapChain.imageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &attachment.imageView), {'R', 205});
        }
    }

    void Renderer::createDepthAttachments()
    {
        for (ImageAttachment &depthAttachment : depthAttachments)
        {
            depthAttachment.image = createImage(swapChain.extent.width, swapChain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, &depthAttachment.memory);

            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = depthAttachment.image;
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = depthFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &depthAttachment.imageView), {'R', 205});
        }
    }

    void Renderer::createShadowDepthAttachment()
    {
        shadowDepthAttachment.image = createImage(SHADOW_MAP_EXTENT.w, SHADOW_MAP_EXTENT.h, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, &shadowDepthAttachment.memory);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = shadowDepthAttachment.image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = depthFormat;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &shadowDepthAttachment.imageView), {'R', 205});
    }

    void Renderer::createSyncObjects()
    {
        renderFinishedSemaphores.resize(swapChain.imageCount);
        VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].imageAvailableSemaphore), {'R', 215});
            vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].drawFence), {'R', 216});
        }

        for (size_t i = 0; i < swapChain.imageCount; i++)
        {
            vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]), {'R', 215});
        }
    }

    Renderer::~Renderer()
    {
        if (device != VK_NULL_HANDLE)
            vkCheck(vkDeviceWaitIdle(device), {'R', 235});

        size_t pipelineCacheSize = 0;
        vkGetPipelineCacheData(device, pipelineCache, &pipelineCacheSize, nullptr);
        std::vector<char> outPipelineCacheData(pipelineCacheSize);
        vkGetPipelineCacheData(device, pipelineCache, &pipelineCacheSize, outPipelineCacheData.data());
        vkDestroyPipelineCache(device, pipelineCache, nullptr);
        std::ofstream file(PIPELINE_CACHE_FILE_NAME, std::ios::binary | std::ios::trunc);
        if (!file.is_open())
            Log::add('R', 110);
        else
            file.write(outPipelineCacheData.data(), static_cast<std::streamsize>(pipelineCacheSize));

        if (textures.sampler)
            vkDestroySampler(device, textures.sampler, nullptr);
        if (textures.descriptorSetLayout)
            vkDestroyDescriptorSetLayout(device, textures.descriptorSetLayout, nullptr);
        for (ImageAttachment &attachment : textures.attachments)
            destroyImageAttachment(attachment);
        for (VkDescriptorPool pool : textures.descriptorPools)
            if (pool)
                vkDestroyDescriptorPool(device, pool, nullptr);

        for (ModelBuffer &modelBuffer : modelBuffers)
            for (MeshBuffer &meshBuffer : modelBuffer.meshBuffers)
                destroyMeshBuffer(meshBuffer);

        for (WidgetBuffer &widgetBuffer : widgetBuffers)
            for (MeshBuffer &meshBuffer : widgetBuffer.meshBuffers)
                destroyMeshBuffer(meshBuffer);

        for (FrameData &frame : frames)
        {
            if (frame.imageAvailableSemaphore)
                vkDestroySemaphore(device, frame.imageAvailableSemaphore, nullptr);
            if (frame.drawFence)
                vkDestroyFence(device, frame.drawFence, nullptr);
        }
        for (size_t i = 0; i < swapChain.imageCount; i++)
            if (renderFinishedSemaphores[i])
                vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);

        auto destroyGraphicsPipeline = [device = device](GraphicsPipeline pipeline)
        {
            if (pipeline.descriptorPool)
                vkDestroyDescriptorPool(device, pipeline.descriptorPool, nullptr);
            if (pipeline.pipeline)
                vkDestroyPipeline(device, pipeline.pipeline, nullptr);
            if (pipeline.layout)
                vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
            if (pipeline.descriptorSetLayout)
                vkDestroyDescriptorSetLayout(device, pipeline.descriptorSetLayout, nullptr);
        };

        destroyGraphicsPipeline(postPipeline);
        if (postSampler)
            vkDestroySampler(device, postSampler, nullptr);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            if (uiUniformBuffers[i])
                vkDestroyBuffer(device, uiUniformBuffers[i], nullptr);
            if (uiUniformBuffersMemory[i])
                vkFreeMemory(device, uiUniformBuffersMemory[i], nullptr);
        }
        destroyGraphicsPipeline(uiPipeline);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            if (cameraUniformBuffer[i])
                vkDestroyBuffer(device, cameraUniformBuffer[i], nullptr);
            if (cameraUniformBufferMemory[i])
                vkFreeMemory(device, cameraUniformBufferMemory[i], nullptr);

            if (lightingUniformBuffer[i])
                vkDestroyBuffer(device, lightingUniformBuffer[i], nullptr);
            if (lightingUniformBufferMemory[i])
                vkFreeMemory(device, lightingUniformBufferMemory[i], nullptr);
        }
        destroyGraphicsPipeline(transparentPipeline);
        destroyGraphicsPipeline(modelPipeline);

        destroyGraphicsPipeline(shadowPipeline);

        destroyImageAttachment(shadowDepthAttachment);

        for (ImageAttachment &depthAttachment : depthAttachments)
            destroyImageAttachment(depthAttachment);

        if (shadowSampler)
            vkDestroySampler(device, shadowSampler, nullptr);

        for (ImageAttachment &attachment : prePostAttachments)
            destroyImageAttachment(attachment);

        destroySwapChain(swapChain);

        if (commandPool)
            vkDestroyCommandPool(device, commandPool, nullptr);

        if (device)
            vkDestroyDevice(device, nullptr);

        if (surface)
            vkDestroySurfaceKHR(instance, surface, nullptr);

        if (instance)
            vkDestroyInstance(instance, nullptr);
    }
}