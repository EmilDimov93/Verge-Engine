// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"
#include "../../shared/version.hpp"
#include "../../shared/local.hpp"

#include <array>

namespace VE
{
    Renderer::Renderer(GLFWwindow *window, Size2 windowSize)
    {
        this->window = window;
        
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain(windowSize);
        createImageViews();
        createDescriptorSetLayout();
        findDepthFormat();
        createShadowDepthBufferImage();
        createShadowSampler();
        createGraphicsPipeline();
        createShadowPipeline();
        createDepthBufferImage();
        createCommandPool();
        createCommandBuffers();
        createTextureSampler();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
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
                Log::add('V', 214);

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

        uint32_t swapChainImageCount;
        vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
        swapChainImages.resize(swapChainImageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages.data());
    }

    void Renderer::createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++)
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

    void Renderer::createGraphicsPipeline()
    {
        std::vector<char> vertexShaderCode = readFile("shaders/vert.spv");
        std::vector<char> fragmentShaderCode = readFile("shaders/frag.spv");

        if (vertexShaderCode.empty() || fragmentShaderCode.empty())
            Log::add('V', 221);

        VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
        VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

        VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
            .pName = "main"};

        VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
            .pName = "main"};

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

        VkVertexInputBindingDescription bindingDescription = {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, col);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, tex);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, norm);

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()};

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE};

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)swapChainExtent.width,
            .height = (float)swapChainExtent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f};

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = VkExtent2D(swapChainExtent.width, swapChainExtent.height)};

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor};

        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f};

        VkPipelineMultisampleStateCreateInfo multiSamplingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE};

        VkPipelineColorBlendAttachmentState colorState = {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments = &colorState};

        std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {descriptorSetLayout, samplerSetLayout};

        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushData);
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange};

        vkCheck(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), {'V', 210});

        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE};

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()};

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &swapChainImageFormat;
        pipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputCreateInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizerCreateInfo,
            .pMultisampleState = &multiSamplingCreateInfo,
            .pDepthStencilState = &depthStencilCreateInfo,
            .pColorBlendState = &colorBlendingCreateInfo,
            .pDynamicState = &dynamicStateCreateInfo,
            .layout = pipelineLayout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1};

        vkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline), {'V', 211});

        vkDestroyShaderModule(device, vertexShaderModule, nullptr);
        vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
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

        shadowDepthFormat = depthFormat;
    }

    void Renderer::createDepthBufferImage()
    {
        depthBufferImage = createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = depthBufferImage;
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
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &depthBufferImageView), {'V', 205});
    }

    void Renderer::createDescriptorSetLayout()
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

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {cameraLayoutBinding, lightingLayoutBinding, shadowLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data()};

        vkCheck(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout), {'V', 217});

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

        vkCheck(vkCreateDescriptorSetLayout(device, &textureLayoutCreateInfo, nullptr, &samplerSetLayout), {'V', 217});
    }

    void Renderer::createCommandPool()
    {
        VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = static_cast<uint32_t>(graphicsQueueFamilyIndex)};

        vkCheck(vkCreateCommandPool(device, &graphicsCommandPoolCreateInfo, nullptr, &graphicsCommandPool), {'V', 208});
    }

    void Renderer::createCommandBuffers()
    {
        commandBuffers.resize(MAX_FRAME_DRAWS);

        VkCommandBufferAllocateInfo commandBufferAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = graphicsCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())};

        vkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, commandBuffers.data()), {'V', 212});
    }

    void Renderer::createSemaphores()
    {
        imageAvailableSemaphores.resize(MAX_FRAME_DRAWS);
        renderFinishedSemaphores.resize(swapChainImages.size());
        VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

        drawFences.resize(MAX_FRAME_DRAWS);
        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
        {
            vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]), {'V', 215});
            vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &drawFences[i]), {'V', 216});
        }

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]), {'V', 215});
        }
    }

    void Renderer::createUniformBuffers()
    {
        VkDeviceSize cameraBufferSize = sizeof(UboCamera);
        VkDeviceSize lightingBufferSize = sizeof(UboLighting);

        cameraUniformBuffer.resize(swapChainImages.size());
        cameraUniformBufferMemory.resize(swapChainImages.size());

        lightingUniformBuffer.resize(swapChainImages.size());
        lightingUniformBufferMemory.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            createBuffer(cameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &cameraUniformBuffer[i], &cameraUniformBufferMemory[i]);
            createBuffer(lightingBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &lightingUniformBuffer[i], &lightingUniformBufferMemory[i]);
        }
    }

    void Renderer::createDescriptorPool()
    {
        VkDescriptorPoolSize uniformPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(cameraUniformBuffer.size() + lightingUniformBuffer.size())};

        VkDescriptorPoolSize shadowPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = static_cast<uint32_t>(swapChainImages.size())};

        std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes = {uniformPoolSize, shadowPoolSize};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(swapChainImages.size()),
            .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
            .pPoolSizes = descriptorPoolSizes.data()};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool), {'V', 219});

        VkDescriptorPoolSize samplerPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_OBJECTS};

        VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = MAX_OBJECTS,
            .poolSizeCount = 1,
            .pPoolSizes = &samplerPoolSize};

        vkCheck(vkCreateDescriptorPool(device, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool), {'V', 219});
    }

    void Renderer::createDescriptorSets()
    {
        descriptorSets.resize(swapChainImages.size());

        std::vector<VkDescriptorSetLayout> setLayouts(swapChainImages.size(), descriptorSetLayout);

        VkDescriptorSetAllocateInfo setAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(swapChainImages.size()),
            .pSetLayouts = setLayouts.data()};

        vkCheck(vkAllocateDescriptorSets(device, &setAllocInfo, descriptorSets.data()), {'V', 220});

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            VkDescriptorBufferInfo cameraBufferInfo = {
                .buffer = cameraUniformBuffer[i],
                .offset = 0,
                .range = sizeof(UboCamera)};

            VkWriteDescriptorSet cameraSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSets[i],
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
                .dstSet = descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &lightingBufferInfo};

            VkDescriptorImageInfo shadowImageInfo = {
                .sampler = shadowSampler,
                .imageView = shadowDepthBufferImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

            VkWriteDescriptorSet shadowSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &shadowImageInfo};

            std::vector<VkWriteDescriptorSet> setWrites = {cameraSetWrite, lightingSetWrite, shadowSetWrite};

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
        }
    }

    void Renderer::createShadowDepthBufferImage()
    {
        shadowDepthBufferImage = createImage(shadowMapExtent.w, shadowMapExtent.h, shadowDepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &shadowDepthBufferImageMemory);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = shadowDepthBufferImage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = shadowDepthFormat;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &shadowDepthBufferImageView), {'V', 205});
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

    void Renderer::createShadowPipeline()
    {
        std::vector<char> code = readFile("shaders/shadowVert.spv");
        if (code.empty())
            Log::add('V', 221);
        VkShaderModule vertexShader = createShaderModule(code);

        VkPipelineShaderStageCreateInfo stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader,
            .pName = "main"};

        VkVertexInputBindingDescription binding = {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
        VkVertexInputAttributeDescription attribute = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)};

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &binding,
            .vertexAttributeDescriptionCount = 1,
            .pVertexAttributeDescriptions = &attribute};

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

        VkViewport viewport = {0, 0, (float)shadowMapExtent.w, (float)shadowMapExtent.h, 0, 1};
        VkRect2D scissors = {{0, 0}, {shadowMapExtent.w, shadowMapExtent.h}};
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissors};

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_FRONT_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_TRUE,
            .depthBiasConstantFactor = 1.25f,
            .depthBiasSlopeFactor = 1.75f,
            .lineWidth = 1.0f};

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS};

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 0};

        VkPushConstantRange pushConstantRange = {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowPushData)};
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange};
        vkCheck(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &shadowPipelineLayout), {'V', 210});

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = 0;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = nullptr;
        pipelineRenderingCreateInfo.depthAttachmentFormat = shadowDepthFormat;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = 1,
            .pStages = &stage,
            .pVertexInputState = &vertexInputStateCreateInfo,
            .pInputAssemblyState = &inputAssemblyStateCreateInfo,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizationStateCreateInfo,
            .pMultisampleState = &multisampleStateCreateInfo,
            .pDepthStencilState = &depthStencilStateCreateInfo,
            .pColorBlendState = &colorBlendStateCreateInfo,
            .layout = shadowPipelineLayout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0};
        vkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &shadowPipeline), {'V', 211});

        vkDestroyShaderModule(device, vertexShader, nullptr);
    }

    Renderer::~Renderer()
    {
        if (device != VK_NULL_HANDLE)
            vkCheck(vkDeviceWaitIdle(device), {'V', 235});

        if (shadowPipeline)
            vkDestroyPipeline(device, shadowPipeline, nullptr);
        if (shadowPipelineLayout)
            vkDestroyPipelineLayout(device, shadowPipelineLayout, nullptr);
        if (shadowSampler)
            vkDestroySampler(device, shadowSampler, nullptr);
        if (shadowDepthBufferImageView)
            vkDestroyImageView(device, shadowDepthBufferImageView, nullptr);
        if (shadowDepthBufferImage)
            vkDestroyImage(device, shadowDepthBufferImage, nullptr);
        if (shadowDepthBufferImageMemory)
            vkFreeMemory(device, shadowDepthBufferImageMemory, nullptr);

        if (samplerSetLayout)
            vkDestroyDescriptorSetLayout(device, samplerSetLayout, nullptr);

        if (textureSampler)
            vkDestroySampler(device, textureSampler, nullptr);

        for (VkImage &image : textureImages)
            vkDestroyImage(device, image, nullptr);

        for (VkDeviceMemory &imageMemory : textureImageMemory)
            vkFreeMemory(device, imageMemory, nullptr);

        for (VkImageView &imageView : textureImageViews)
            vkDestroyImageView(device, imageView, nullptr);

        for (ModelBuffer &modelBuffer : modelBuffers)
        {
            for (MeshBuffer &meshBuffer : modelBuffer.meshBuffers)
                destroyMeshBuffer(meshBuffer);
        }

        if (depthBufferImageView)
            vkDestroyImageView(device, depthBufferImageView, nullptr);

        if (depthBufferImage)
            vkDestroyImage(device, depthBufferImage, nullptr);

        if (depthBufferImageMemory)
            vkFreeMemory(device, depthBufferImageMemory, nullptr);

        if (descriptorPool)
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        if (samplerDescriptorPool)
            vkDestroyDescriptorPool(device, samplerDescriptorPool, nullptr);
        if (descriptorSetLayout)
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        for (size_t i = 0; i < swapChainImages.size(); i++)
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

        for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
        {
            if (imageAvailableSemaphores[i])
                vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            if (drawFences[i])
                vkDestroyFence(device, drawFences[i], nullptr);
        }

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            if (renderFinishedSemaphores[i])
                vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        }

        if (graphicsCommandPool)
            vkDestroyCommandPool(device, graphicsCommandPool, nullptr);

        if (graphicsPipeline)
            vkDestroyPipeline(device, graphicsPipeline, nullptr);

        if (pipelineLayout)
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        for (VkImageView iv : swapChainImageViews)
            if (iv)
                vkDestroyImageView(device, iv, nullptr);
        swapChainImageViews.clear();

        if (swapChain)
            vkDestroySwapchainKHR(device, swapChain, nullptr);

        if (device)
            vkDestroyDevice(device, nullptr);

        if (surface)
            vkDestroySurfaceKHR(instance, surface, nullptr);

        if (instance)
            vkDestroyInstance(instance, nullptr);
    }
}