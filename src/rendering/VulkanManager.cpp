// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "VulkanManager.hpp"

#include "../Log.hpp"
#include "../version.hpp"
#include "Camera.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <fstream>

const uint8_t MAX_FRAME_DRAWS = 2;

VulkanManager::VulkanManager(GLFWwindow *window, Size2 windowSize)
{
    createInstance();
    createSurface(window);
    pickPhysicalDevice();

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties); // test

    createLogicalDevice();
    createSwapChain(windowSize);
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createPushConstantRange();
    createGraphicsPipeline();
    createDepthBufferImage();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createSemaphores();

    Log::add('V', 000);
}

void VulkanManager::createInstance()
{
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Verge Engine",
        .applicationVersion = VK_MAKE_VERSION(0, 0, VERGE_ENGINE_VERSION),
        .apiVersion = VK_API_VERSION_1_3};

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions};

    vkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), {'V', 200});
}

void VulkanManager::createSurface(GLFWwindow *window)
{
    vkCheck(glfwCreateWindowSurface(instance, window, nullptr, &surface), {'V', 201});
}

int VulkanManager::rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures feats;
    vkGetPhysicalDeviceProperties(device, &props);
    vkGetPhysicalDeviceFeatures(device, &feats);

    int score = 0;

    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    score += props.limits.maxImageDimension2D;

    if (!feats.geometryShader)
    {
        return 0;
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int32_t graphicsFamily = -1;
    int32_t presentationFamily = -1;
    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsFamily = i;
        }

        VkBool32 presentationSupport = false;
        vkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport), {'V', 214});

        if (queueFamily.queueCount > 0 && presentationSupport)
        {
            presentationFamily = i;
        }

        if (graphicsFamily >= 0 && presentationFamily >= 0)
        {
            break;
        }

        i++;
    }

    bool supportsRequiredExtentions = true;
    if (!supportsRequiredExtentions)
    {
        return 0;
    }

    if (graphicsFamily < 0 || presentationFamily < 0)
    {
        return 0;
    }

    return score;
}

void VulkanManager::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr), {'V', 202});
    if (deviceCount == 0)
    {
        Log::add('V', 202);
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()), {'V', 202});

    int bestScore = 0;
    for (auto device : devices)
    {
        int score = rateDevice(device, surface);
        if (score > bestScore)
        {
            bestScore = score;
            physicalDevice = device;
        }
    }

    if (bestScore == 0)
    {
        Log::add('V', 202);
    }
}

void VulkanManager::createLogicalDevice()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    graphicsQueueFamilyIndex = 0;
    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueFamilyIndex = i;
            break;
        }
        i++;
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = static_cast<uint32_t>(graphicsQueueFamilyIndex),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions,
        .pEnabledFeatures = &deviceFeatures};

    vkCheck(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device), {'V', 203});

    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
    presentQueue = graphicsQueue;
}

void VulkanManager::createSwapChain(Size2 windowSize)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    swapChainImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    swapChainExtent = {windowSize.w, windowSize.h};

    VkSwapchainCreateInfoKHR createInfo = {
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

    vkCheck(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain), {'V', 204});

    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
    swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages.data());
}

void VulkanManager::createImageViews()
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

VkShaderModule VulkanManager::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data())};

    VkShaderModule shaderModule;

    vkCheck(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule), {'V', 209});

    return shaderModule;
}

static std::vector<char> readFile(const std::string &fileName){
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);

    if(!file.is_open()){
        return {};
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);

    file.read(fileBuffer.data(), fileSize);

    file.close();

    return fileBuffer;
}

void VulkanManager::createGraphicsPipeline()
{
    std::vector<char> vertexShaderCode = readFile("shaders/vert.spv");
    std::vector<char> fragmentShaderCode = readFile("shaders/frag.spv");

    if (vertexShaderCode.empty() || fragmentShaderCode.empty())
    {
        Log::add('V', 221);
    }

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

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, col);

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

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
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

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizerCreateInfo,
        .pMultisampleState = &multiSamplingCreateInfo,
        .pDepthStencilState = &depthStencilCreateInfo,
        .pColorBlendState = &colorBlendingCreateInfo,
        .pDynamicState = nullptr,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1};

    vkCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline), {'V', 211});

    vkDestroyShaderModule(device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
}

// Duplicate: exists in Mesh.cpp already
uint32_t findMemoryTypeIndex1(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((allowedTypes & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
        {
            return i;
        }
    }

    std::cout << "No suitable memory type index";
    return -1;
}

void VulkanManager::createDepthBufferImage()
{
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = swapChainExtent.width;
    imageCreateInfo.extent.height = swapChainExtent.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = depthFormat;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCheck(vkCreateImage(device, &imageCreateInfo, nullptr, &depthBufferImage), {'V', 222});

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, depthBufferImage, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryTypeIndex1(physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};

    vkCheck(vkAllocateMemory(device, &memoryAllocInfo, nullptr, &depthBufferImageMemory), {'V', 222});

    vkBindImageMemory(device, depthBufferImage, depthBufferImageMemory, 0);

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

void VulkanManager::createRenderPass()
{
    VkAttachmentDescription colorAttachment{
        .format = swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    depthFormat = VK_FORMAT_UNDEFINED;

    std::vector<VkFormat> formats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT};

    for (VkFormat f : formats)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, f, &props);

        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depthFormat = f;
            break;
        }
    }

    if (depthFormat == VK_FORMAT_UNDEFINED)
    {
        Log::add('V', 223);
    }

    VkAttachmentDescription depthAttachment = {
        .format = depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthAttachmentRef = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef};

    std::vector<VkSubpassDependency> subpassDependencies(2);

    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    subpassDependencies[0].dependencyFlags = 0;

    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    subpassDependencies[1].dependencyFlags = 0;

    std::vector<VkAttachmentDescription> renderPassAttachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(renderPassAttachments.size()),
        .pAttachments = renderPassAttachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t>(subpassDependencies.size()),
        .pDependencies = subpassDependencies.data()};

    vkCheck(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass), {'V', 206});
}

void VulkanManager::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding vpLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr};

    std::vector<VkDescriptorSetLayoutBinding> vpLayoutBindings = {vpLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(vpLayoutBindings.size()),
        .pBindings = vpLayoutBindings.data()};

    vkCheck(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout), {'V', 217});
}

void VulkanManager::createPushConstantRange()
{
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(Model);
}

void VulkanManager::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {swapChainImageViews[i], depthBufferImageView};
        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .layers = 1};

        vkCheck(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]), {'V', 207});
    }
}

void VulkanManager::createCommandPool()
{
    VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = static_cast<uint32_t>(graphicsQueueFamilyIndex)};

    vkCheck(vkCreateCommandPool(device, &graphicsCommandPoolCreateInfo, nullptr, &graphicsCommandPool), {'V', 208});
}

void VulkanManager::createCommandBuffers()
{
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = graphicsCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())};

    vkCheck(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, commandBuffers.data()), {'V', 212});
}

void VulkanManager::updateUniformBuffers(uint32_t imageIndex)
{
    struct UboViewProjection
    {
        glm::mat4 projection;
        glm::mat4 view;
    }uboViewProjection;

    uboViewProjection.projection = Camera::getProjectionMatrix();
    uboViewProjection.view = Camera::getViewMatrix();

    void *data;
    vkCheck(vkMapMemory(device, vpUniformBufferMemory[imageIndex], 0, sizeof(UboViewProjection), 0, &data), {'V', 236});
    memcpy(data, &uboViewProjection, sizeof(UboViewProjection));
    vkUnmapMemory(device, vpUniformBufferMemory[imageIndex]);
}

void VulkanManager::recordCommands(uint32_t currentImage, const std::vector<Mesh>& meshes)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    vkCheck(vkBeginCommandBuffer(commandBuffers[currentImage], &commandBufferBeginInfo), {'V', 213});

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapChainFramebuffers[currentImage];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.878f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil.depth = 1.0f;
    renderPassBeginInfo.pClearValues = clearValues.data();
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

    vkCmdBeginRenderPass(commandBuffers[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    for (size_t j = 0; j < meshes.size(); j++)
    {
        VkBuffer vertexBuffers[] = {meshes[j].getVertexBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[currentImage], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffers[currentImage], meshes[j].getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

        Model m = meshes[j].getModel();
        vkCmdPushConstants(commandBuffers[currentImage], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Model), &m);

        vkCmdBindDescriptorSets(commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentImage], 0, nullptr);

        vkCmdDrawIndexed(commandBuffers[currentImage], meshes[j].getIndexCount(), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffers[currentImage]);

    vkCheck(vkEndCommandBuffer(commandBuffers[currentImage]), {'V', 213});
}

void VulkanManager::createSemaphores()
{
    imageAvailableSemaphores.resize(MAX_FRAME_DRAWS);
    renderFinishedSemaphores.resize(MAX_FRAME_DRAWS);
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    drawFences.resize(MAX_FRAME_DRAWS);
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
    {
        vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]), {'V', 215});
        vkCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]), {'V', 215});
        vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &drawFences[i]), {'V', 216});
    }
}

// Duplicate: exists in Mesh.cpp already
#define VK_CHECK1(res)         \
    do                         \
    {                          \
        if (res != VK_SUCCESS) \
            return res;        \
    } while (0)

// Duplicate: exists in Mesh.cpp already
VkResult createBuffer1(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory)
{
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = bufferUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    VK_CHECK1(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryTypeIndex1(physicalDevice, memRequirements.memoryTypeBits, bufferPropertyFlags)};

    VK_CHECK1(vkAllocateMemory(device, &memoryAllocInfo, nullptr, bufferMemory));

    VK_CHECK1(vkBindBufferMemory(device, *buffer, *bufferMemory, 0));

    return VK_SUCCESS;
}

void VulkanManager::createUniformBuffers()
{
    VkDeviceSize vpBufferSize = sizeof(glm::mat4) * 2;

    vpUniformBuffer.resize(swapChainImages.size());
    vpUniformBufferMemory.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        vkCheck(createBuffer1(physicalDevice, device, vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vpUniformBuffer[i], &vpUniformBufferMemory[i]), {'V', 218});
    }
}

void VulkanManager::createDescriptorPool()
{
    VkDescriptorPoolSize vpPoolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t>(vpUniformBuffer.size())};

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {vpPoolSize};

    VkDescriptorPoolCreateInfo poolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(swapChainImages.size()),
        .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
        .pPoolSizes = descriptorPoolSizes.data()};

    vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool), {'V', 219});
}

void VulkanManager::createDescriptorSets()
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
        VkDescriptorBufferInfo vpBufferInfo = {
            .buffer = vpUniformBuffer[i],
            .offset = 0,
            .range = sizeof(glm::mat4) * 2};

        VkWriteDescriptorSet vpSetWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &vpBufferInfo};

        std::vector<VkWriteDescriptorSet> setWrites = {vpSetWrite};

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}

void VulkanManager::drawFrame(const std::vector<Mesh>& meshes)
{
    vkCheck(vkWaitForFences(device, 1, &drawFences[currentFrame], VK_TRUE, UINT64_MAX), {'V', 231});
    vkCheck(vkResetFences(device, 1, &drawFences[currentFrame]), {'V', 232});

    uint32_t imageIndex;
    vkCheck(vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex), {'V', 230});

    recordCommands(imageIndex, meshes);
    updateUniformBuffers(imageIndex);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAvailableSemaphores[currentFrame],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffers[imageIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderFinishedSemaphores[currentFrame]};

    vkCheck(vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]), {'V', 233});

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinishedSemaphores[currentFrame],
        .swapchainCount = 1,
        .pSwapchains = &swapChain,
        .pImageIndices = &imageIndex};

    vkCheck(vkQueuePresentKHR(presentQueue, &presentInfo), {'V', 234});

    currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

void VulkanManager::vkCheck(VkResult res, ErrorCode errorCode)
{
    switch (res)
    {
    case VK_SUCCESS:
        return;
    case VK_NOT_READY:
        Log::add('V', 100);
        break;
    case VK_TIMEOUT:
        Log::add('V', 101);
        break;
    case VK_SUBOPTIMAL_KHR:
        Log::add('V', 102);
        break;
    case VK_EVENT_SET:
        Log::add('V', 103);
        break;
    case VK_EVENT_RESET:
        Log::add('V', 104);
        break;
    default:
        Log::add(errorCode.letter, errorCode.number);
    }
}

VulkanContext VulkanManager::getContext()
{
    VulkanContext ctx = {
        .physicalDevice = physicalDevice,
        .device = device,
        .graphicsQueue = graphicsQueue,
        .graphicsCommandPool = graphicsCommandPool
    };

    return ctx;
}

VulkanManager::~VulkanManager()
{
    if (device != VK_NULL_HANDLE)
        vkCheck(vkDeviceWaitIdle(device), {'V', 235});

    if (depthBufferImageView)
        vkDestroyImageView(device, depthBufferImageView, nullptr);

    if (depthBufferImage)
        vkDestroyImage(device, depthBufferImage, nullptr);

    if (depthBufferImageMemory)
        vkFreeMemory(device, depthBufferImageMemory, nullptr);

    if (descriptorPool)
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    if (descriptorSetLayout)
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        if (vpUniformBuffer[i])
            vkDestroyBuffer(device, vpUniformBuffer[i], nullptr);
        if (vpUniformBufferMemory[i])
            vkFreeMemory(device, vpUniformBufferMemory[i], nullptr);
    }

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
    {
        if (renderFinishedSemaphores[i])
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        if (imageAvailableSemaphores[i])
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        if (drawFences[i])
            vkDestroyFence(device, drawFences[i], nullptr);
    }

    if (graphicsCommandPool)
        vkDestroyCommandPool(device, graphicsCommandPool, nullptr);

    for (auto fb : swapChainFramebuffers)
        if (fb)
            vkDestroyFramebuffer(device, fb, nullptr);
    swapChainFramebuffers.clear();

    if (graphicsPipeline)
        vkDestroyPipeline(device, graphicsPipeline, nullptr);

    if (pipelineLayout)
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    if (renderPass)
        vkDestroyRenderPass(device, renderPass, nullptr);

    for (auto iv : swapChainImageViews)
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