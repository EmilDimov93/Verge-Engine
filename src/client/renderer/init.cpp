// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "../../shared/log.hpp"
#include "../../shared/version.hpp"
#include "../../shared/local.hpp"

#include <fstream>

namespace VE
{
    Renderer::Renderer(GLFWwindow *window, glm::uvec2 windowSize) : gBuffer(currentFrame), imageAvailableSemaphore(currentFrame), drawFence(currentFrame), commandBuffer(currentFrame), cameraUniformBuffer(currentFrame), cameraUniformBufferMemory(currentFrame), lightingUniformBuffer(currentFrame), lightingUniformBufferMemory(currentFrame), uiUniformBuffer(currentFrame), uiUniformBufferMemory(currentFrame)
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

        createShadowSampler();
        createFallbackShadowAttachment();

        createPipelineCache();

        createTextureSampler();
        createTextureDescriptorSetLayout();
        createFallbackTexture();

        createShadowPipeline();

        createGeometryBuffers();
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

        bool foundGraphicsQueue = false;
        bool foundTransferQueue = false;
        for (size_t i = 0; i < queueFamilies.size(); i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (!foundGraphicsQueue)
                {
                    graphicsQueueFamilyIndex = i;
                    foundGraphicsQueue = true;
                }
            }
            else if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && queueFamilies[i].queueCount > 0)
            {
                if (!foundTransferQueue)
                {
                    transferQueueFamilyIndex = i;
                    foundTransferQueue = true;
                }
            }
        }

        if (!foundGraphicsQueue || !foundTransferQueue)
            Log::add('R', 241);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float queuePriority = 1.f;
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
            if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                swapChain.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
                break;
            }
        }

        if (swapChain.imageFormat == VK_FORMAT_UNDEFINED)
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

    void Renderer::createSwapChain(glm::uvec2 windowSize)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        swapChain.extent = {windowSize.x, windowSize.y};

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
            commandBuffer.arr[i] = commandBuffers[i];
    }

    void Renderer::createPrePostImages()
    {
        prePostAttachments.resize(swapChain.imageCount);
        for (ImageAttachment &attachment : prePostAttachments)
        {
            attachment.image = createImage(swapChain.extent.width, swapChain.extent.height, HDR_COLOR_FORMAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, &attachment.memory);

            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = attachment.image;
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = HDR_COLOR_FORMAT;
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

    void Renderer::createGeometryBuffers()
    {
        for (GeometryBuffer &geometryBuffer : gBuffer.arr)
        {
            geometryBuffer.depth = createAttachment(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, swapChain.extent);
            geometryBuffer.normal = createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, swapChain.extent);
            geometryBuffer.metallic = createAttachment(VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, swapChain.extent);
            geometryBuffer.roughness = createAttachment(VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, swapChain.extent);
        }
    }

    void Renderer::createShadowSampler()
    {
        VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .anisotropyEnable = VK_FALSE,
            .compareEnable = VK_TRUE,
            .compareOp = VK_COMPARE_OP_LESS,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE};

        vkCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &shadowSampler), {'R', 218});
    }

    void Renderer::createFallbackShadowAttachment()
    {
        ImageAttachment attachment;

        attachment.image = createImage(1, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, &attachment.memory);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = attachment.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkCheck(vkCreateImageView(device, &viewInfo, nullptr, &attachment.imageView), {'R', 205});

        FenceGuard fence(device);

        transitionImageLayout(device, graphicsQueue, commandPool, attachment.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, graphicsQueueMutex, fence);

        shadowMaps[0].depthAttachment = attachment;
        shadowMapCount = 1;
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
            vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore.arr[i]), {'R', 215});
            vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &drawFence.arr[i]), {'R', 216});
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

        for (VkSemaphore &imageAvailableSemaphore : imageAvailableSemaphore.arr)
            if (imageAvailableSemaphore)
                vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        for (VkFence &drawFence : drawFence.arr)
            if (drawFence)
                vkDestroyFence(device, drawFence, nullptr);

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
            if (uiUniformBuffer.arr[i])
                vkDestroyBuffer(device, uiUniformBuffer.arr[i], nullptr);
            if (uiUniformBufferMemory.arr[i])
                vkFreeMemory(device, uiUniformBufferMemory.arr[i], nullptr);
        }
        destroyGraphicsPipeline(uiPipeline);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            if (cameraUniformBuffer.arr[i])
                vkDestroyBuffer(device, cameraUniformBuffer.arr[i], nullptr);
            if (cameraUniformBufferMemory.arr[i])
                vkFreeMemory(device, cameraUniformBufferMemory.arr[i], nullptr);

            if (lightingUniformBuffer.arr[i])
                vkDestroyBuffer(device, lightingUniformBuffer.arr[i], nullptr);
            if (lightingUniformBufferMemory.arr[i])
                vkFreeMemory(device, lightingUniformBufferMemory.arr[i], nullptr);
        }
        destroyGraphicsPipeline(transparentPipeline);
        destroyGraphicsPipeline(modelPipeline);

        destroyGraphicsPipeline(shadowPipeline);

        for (GeometryBuffer &gBuffers : gBuffer.arr)
        {
            destroyImageAttachment(gBuffers.depth);
            destroyImageAttachment(gBuffers.normal);
            destroyImageAttachment(gBuffers.metallic);
            destroyImageAttachment(gBuffers.roughness);
        }

        if (shadowSampler)
            vkDestroySampler(device, shadowSampler, nullptr);

        for (size_t i = 0; i < shadowMapCount; i++)
            destroyImageAttachment(shadowMaps[i].depthAttachment);

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