// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"
#include "../../shared/version.hpp"
#include "../../shared/local.hpp"

namespace VE
{
    Renderer::Renderer(GLFWwindow *window, Size2 windowSize)
    {
        this->window = window;

        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();

        createCommandPool();
        createCommandBuffers();

        createSwapChain(windowSize);

        createPrePostImages();

        findDepthFormat();
        createShadowSampler();
        createTextureSampler();

        createDepthAttachment();
        createShadowDepthAttachment();

        createShadowPipeline();

        createModelDescriptorSetLayout();
        createModelPipeline();
        createModelUniformBuffers();
        createModelDescriptorPool();
        createModelDescriptorSets();

        createTransparentPipeline();

        createPostDescriptorPool();
        createPostSampler();
        createPostDescriptorSetLayout();
        createPostPipeline();
        createPostDescriptorSets();

        createUIDescriptorSetLayout();
        createUIPipeline();
        createUIUniformBuffers();
        createUIDescriptorPool();
        createUIDescriptorSets();

        createSemaphores();

        createFallbackTexture();

        Log::add('V', 000);
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

        vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), {'V', 200});
    }

    void Renderer::createSurface()
    {
        vkCheck(glfwCreateWindowSurface(instance, window, nullptr, &surface), {'V', 201});
    }

    void Renderer::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr), {'V', 202});
        if (deviceCount == 0)
            Log::add('V', 202);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()), {'V', 202});

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
            Log::add('V', 202);
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

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &dynamicRenderingFeatures,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = 1,
            .ppEnabledExtensionNames = &swapChainExtention,
            .pEnabledFeatures = &deviceFeatures};

        vkCheck(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device), {'V', 203});

        vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(device, transferQueueFamilyIndex, 0, &transferQueue);
        presentQueue = graphicsQueue;
    }

    void Renderer::createSwapChain(Size2 windowSize)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        swapChainImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
        swapChainExtent = {windowSize.w, windowSize.h};

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = capabilities.minImageCount + (capabilities.minImageCount != capabilities.maxImageCount),
            .imageFormat = swapChainImageFormat,
            .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = swapChainExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE};

        vkCheck(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain), {'V', 204});

        vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
        swapChainImages.resize(swapChainImageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages.data());

        swapChainImageViews.resize(swapChainImageCount);
        for (size_t i = 0; i < swapChainImageCount; i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = swapChainImages[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = swapChainImageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]), {'V', 205});
        }
    }

    void Renderer::createPrePostImages()
    {
        prePostAttachments.resize(swapChainImageCount);
        for (ImageAttachment& attachment : prePostAttachments)
        {
            attachment.image = createImage(swapChainExtent.width, swapChainExtent.height, swapChainImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, &attachment.memory);

            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = attachment.image;
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = swapChainImageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &attachment.imageView), {'V', 205});
        }
    }

    void Renderer::findDepthFormat()
    {
        depthFormat = VK_FORMAT_UNDEFINED;

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
            Log::add('V', 223);
    }

    void Renderer::createDepthAttachment()
    {
        depthAttachment.image = createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthAttachment.memory);

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
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &depthAttachment.imageView), {'V', 205});
    }

    void Renderer::createModelDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding cameraLayoutBinding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutBinding lightingLayoutBinding = {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutBinding shadowLayoutBinding = {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr};

        std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings = {cameraLayoutBinding, lightingLayoutBinding, shadowLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data()};

        vkCheck(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &modelPipeline.descriptorSetLayout), {'V', 217});

        VkDescriptorSetLayoutBinding samplerLayoutBinding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &samplerLayoutBinding};

        vkCheck(vkCreateDescriptorSetLayout(device, &textureLayoutCreateInfo, nullptr, &textures.descriptorSetLayout), {'V', 217});
    }

    void Renderer::createCommandPool()
    {
        VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = static_cast<uint32_t>(graphicsQueueFamilyIndex)};

        vkCheck(vkCreateCommandPool(device, &graphicsCommandPoolCreateInfo, nullptr, &commandPool), {'V', 208});
    }

    void Renderer::createCommandBuffers()
    {
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())};

        vkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, commandBuffers.data()), {'V', 212});
    }

    void Renderer::createSemaphores()
    {
        renderFinishedSemaphores.resize(swapChainImageCount);
        VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]), {'V', 215});
            vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &drawFences[i]), {'V', 216});
        }

        for (size_t i = 0; i < swapChainImageCount; i++)
        {
            vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]), {'V', 215});
        }
    }

    void Renderer::createModelUniformBuffers()
    {
        VkDeviceSize cameraBufferSize = sizeof(UboCamera);
        VkDeviceSize lightingBufferSize = sizeof(UboLighting);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            createBuffer(cameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &cameraUniformBuffer[i], &cameraUniformBufferMemory[i]);
            createBuffer(lightingBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &lightingUniformBuffer[i], &lightingUniformBufferMemory[i]);
        }
    }

    void Renderer::createModelDescriptorPool()
    {
        VkDescriptorPoolSize uniformPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(cameraUniformBuffer.size() + lightingUniformBuffer.size())};

        VkDescriptorPoolSize shadowPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = FRAMES_IN_FLIGHT};

        std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes = {uniformPoolSize, shadowPoolSize};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = FRAMES_IN_FLIGHT,
            .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
            .pPoolSizes = descriptorPoolSizes.data()};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &modelPipeline.descriptorPool), {'V', 219});
    }

    void Renderer::createModelDescriptorSets()
    {
        modelPipeline.descriptorSets.resize(FRAMES_IN_FLIGHT);

        std::vector<VkDescriptorSetLayout> setLayouts(FRAMES_IN_FLIGHT, modelPipeline.descriptorSetLayout);

        VkDescriptorSetAllocateInfo setAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = modelPipeline.descriptorPool,
            .descriptorSetCount = FRAMES_IN_FLIGHT,
            .pSetLayouts = setLayouts.data()};

        vkCheck(vkAllocateDescriptorSets(device, &setAllocInfo, modelPipeline.descriptorSets.data()), {'V', 220});

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo cameraBufferInfo = {
                .buffer = cameraUniformBuffer[i],
                .offset = 0,
                .range = sizeof(UboCamera)};

            VkWriteDescriptorSet cameraSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = modelPipeline.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &cameraBufferInfo};

            VkDescriptorBufferInfo lightingBufferInfo = {
                .buffer = lightingUniformBuffer[i],
                .offset = 0,
                .range = sizeof(UboLighting)};

            VkWriteDescriptorSet lightingSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = modelPipeline.descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &lightingBufferInfo};

            VkDescriptorImageInfo shadowImageInfo = {
                .sampler = shadowSampler,
                .imageView = shadowDepthAttachment.imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

            VkWriteDescriptorSet shadowSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = modelPipeline.descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &shadowImageInfo};

            std::vector<VkWriteDescriptorSet> setWrites = {cameraSetWrite, lightingSetWrite, shadowSetWrite};

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
        }
    }

    void Renderer::createShadowDepthAttachment()
    {
        shadowDepthAttachment.image = createImage(SHADOW_MAP_EXTENT.w, SHADOW_MAP_EXTENT.h, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &shadowDepthAttachment.memory);

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
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &shadowDepthAttachment.imageView), {'V', 205});
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
        vkCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &shadowSampler), {'V', 218});
    }

    void Renderer::createUIDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uiLayoutBinding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutCreateInfo uiLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &uiLayoutBinding};

        vkCheck(vkCreateDescriptorSetLayout(device, &uiLayoutCreateInfo, nullptr, &uiPipeline.descriptorSetLayout), {'V', 217});
    }

    void Renderer::createUIUniformBuffers()
    {
        VkDeviceSize uiUniformBufferSize = sizeof(UboUI);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            createBuffer(uiUniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uiUniformBuffers[i], &uiUniformBuffersMemory[i]);
    }

    void Renderer::createUIDescriptorPool()
    {
        VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(uiUniformBuffers.size())};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = FRAMES_IN_FLIGHT,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &uiPipeline.descriptorPool), {'V', 219});
    }

    void Renderer::createUIDescriptorSets()
    {
        uiPipeline.descriptorSets.resize(FRAMES_IN_FLIGHT);

        std::vector<VkDescriptorSetLayout> setLayouts(FRAMES_IN_FLIGHT, uiPipeline.descriptorSetLayout);

        VkDescriptorSetAllocateInfo setAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = uiPipeline.descriptorPool,
            .descriptorSetCount = FRAMES_IN_FLIGHT,
            .pSetLayouts = setLayouts.data()};

        vkCheck(vkAllocateDescriptorSets(device, &setAllocInfo, uiPipeline.descriptorSets.data()), {'V', 220});

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo uiBufferInfo = {
                .buffer = uiUniformBuffers[i],
                .offset = 0,
                .range = sizeof(UboUI)};

            VkWriteDescriptorSet uiSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = uiPipeline.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &uiBufferInfo};

            vkUpdateDescriptorSets(device, 1, &uiSetWrite, 0, nullptr);
        }
    }

    void Renderer::createPostSampler()
    {
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.anisotropyEnable = VK_FALSE;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 0.0f;

        vkCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &postSampler), {'V', 226});
    }

    void Renderer::createPostDescriptorPool()
    {
        VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = static_cast<uint32_t>(swapChainImageCount)};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(swapChainImageCount),
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &postPipeline.descriptorPool), {'V', 219});
    }

    void Renderer::createPostDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding sceneColorBinding{};
        sceneColorBinding.binding = 0;
        sceneColorBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sceneColorBinding.descriptorCount = 1;
        sceneColorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        sceneColorBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &sceneColorBinding;

        vkCheck(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &postPipeline.descriptorSetLayout), {'V', 217});
    }

    void Renderer::createPostDescriptorSets()
    {
        const size_t imageCount = swapChainImageCount;

        std::vector<VkDescriptorSetLayout> layouts(imageCount, postPipeline.descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = postPipeline.descriptorPool;
        allocateInfo.descriptorSetCount = static_cast<uint32_t>(imageCount);
        allocateInfo.pSetLayouts = layouts.data();

        postPipeline.descriptorSets.resize(imageCount);
        vkCheck(vkAllocateDescriptorSets(device, &allocateInfo, postPipeline.descriptorSets.data()), {'V', 220});

        updatePostDescriptorSets();
    }

    void Renderer::updatePostDescriptorSets()
    {
        for (size_t i = 0; i < postPipeline.descriptorSets.size(); i++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = prePostAttachments[i].imageView;
            imageInfo.sampler = postSampler;

            VkWriteDescriptorSet postSetWrite{};
            postSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            postSetWrite.dstSet = postPipeline.descriptorSets[i];
            postSetWrite.dstBinding = 0;
            postSetWrite.dstArrayElement = 0;
            postSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            postSetWrite.descriptorCount = 1;
            postSetWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &postSetWrite, 0, nullptr);
        }
    }

    void Renderer::destroyMeshBuffer(MeshBuffer &meshBuffer) const
    {
        if (meshBuffer.vertexBuffer)
            vkDestroyBuffer(device, meshBuffer.vertexBuffer, nullptr);
        if (meshBuffer.vertexBufferMemory)
            vkFreeMemory(device, meshBuffer.vertexBufferMemory, nullptr);

        if (meshBuffer.indexBuffer)
            vkDestroyBuffer(device, meshBuffer.indexBuffer, nullptr);
        if (meshBuffer.indexBufferMemory)
            vkFreeMemory(device, meshBuffer.indexBufferMemory, nullptr);

        meshBuffer.vertexBuffer = VK_NULL_HANDLE;
        meshBuffer.vertexBufferMemory = VK_NULL_HANDLE;
        meshBuffer.indexBuffer = VK_NULL_HANDLE;
        meshBuffer.indexBufferMemory = VK_NULL_HANDLE;
    }

    Renderer::~Renderer()
    {
        if (device != VK_NULL_HANDLE)
            vkCheck(vkDeviceWaitIdle(device), {'V', 235});

        if (textures.sampler)
            vkDestroySampler(device, textures.sampler, nullptr);
        if (textures.descriptorSetLayout)
            vkDestroyDescriptorSetLayout(device, textures.descriptorSetLayout, nullptr);
        for(ImageAttachment& attachment : textures.attachments)
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

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            if (imageAvailableSemaphores[i])
                vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            if (drawFences[i])
                vkDestroyFence(device, drawFences[i], nullptr);
        }
        for (size_t i = 0; i < swapChainImageCount; i++)
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

        destroyImageAttachment(depthAttachment);

        if (shadowSampler)
            vkDestroySampler(device, shadowSampler, nullptr);

        for (ImageAttachment& attachment : prePostAttachments)
            destroyImageAttachment(attachment);

        for (VkImageView iv : swapChainImageViews)
            if (iv)
                vkDestroyImageView(device, iv, nullptr);
        swapChainImageViews.clear();

        if (swapChain)
            vkDestroySwapchainKHR(device, swapChain, nullptr);

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