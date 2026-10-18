// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "../../shared/version.hpp"
#include "../../shared/local.hpp"

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

    uint32_t Renderer::rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &props);
        vkGetPhysicalDeviceFeatures(device, &features);

        int score = 0;

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;

        score += props.limits.maxImageDimension2D;

        if (!features.samplerAnisotropy)
            return 0;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int32_t graphicsFamily = -1;
        int32_t presentationFamily = -1;
        int i = 0;
        for (const VkQueueFamilyProperties &queueFamily : queueFamilies)
        {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsFamily = i;

            VkBool32 presentationSupport = false;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport) != VK_SUCCESS)
                Log::add('R', 214);

            if (queueFamily.queueCount > 0 && presentationSupport)
                presentationFamily = i;

            if (graphicsFamily >= 0 && presentationFamily >= 0)
                break;

            i++;
        }

        if (graphicsFamily < 0 || presentationFamily < 0)
            return 0;

        return score;
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

            constexpr VkFormatFeatureFlags requiredDepthFeatures = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
            if ((properties.optimalTilingFeatures & requiredDepthFeatures) == requiredDepthFeatures)
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

    void Renderer::createPipelineCache()
    {
        std::vector<char> pipelineCacheData = readFile(PIPELINE_CACHE_FILE_NAME);

        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        if (!pipelineCacheData.empty())
        {
            pipelineCacheCreateInfo.initialDataSize = pipelineCacheData.size();
            pipelineCacheCreateInfo.pInitialData = pipelineCacheData.data();
        }
        vkCheck(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache), {'R', 227});
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
            destroyImageAttachment(gBuffers.baseColor);
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