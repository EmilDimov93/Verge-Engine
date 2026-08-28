// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "../../shared/log.hpp"

#include <fstream>

namespace VE
{
    constexpr VkVertexInputBindingDescription DEFAULT_BINDING_DESCRIPTION = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    constexpr VkPipelineInputAssemblyStateCreateInfo DEFAULT_INPUT_ASSEMBLY_CREATE_INFO = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    constexpr VkPipelineMultisampleStateCreateInfo DEFAULT_MULTISAMPLE_CREATE_INFO = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE};

    constexpr std::array<VkDynamicState, 2> DEFAULT_DYNAMIC_STATE = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    constexpr VkPipelineDynamicStateCreateInfo DEFAULT_DYNAMIC_STATE_CREATE_INFO = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = DEFAULT_DYNAMIC_STATE.data()};

    constexpr VkPipelineViewportStateCreateInfo DEFAULT_VIEWPORT_STATE_CREATE_INFO = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1};

    [[nodiscard]] static std::vector<char> readFile(const std::string &fileName)
    {
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);

        if (!file.is_open())
            return {};

        size_t fileSize = (size_t)file.tellg();

        std::vector<char> fileBuffer(fileSize);

        file.seekg(0);

        file.read(fileBuffer.data(), fileSize);

        file.close();

        return fileBuffer;
    }

    class ShaderStageGuard
    {
    public:
        ShaderStageGuard(std::string path, VkShaderStageFlagBits stageFlag, VkDevice device) : device(device)
        {
            std::vector<char> code = readFile(path);

            if (code.empty())
                Log::add('R', 221);

            VkShaderModuleCreateInfo shaderModuleCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t *>(code.data())};

            Renderer::vkCheck(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &module), {'R', 209});

            stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stage.stage = stageFlag;
            ;
            stage.module = module;
            stage.pName = "main";
        };

        ~ShaderStageGuard()
        {
            vkDestroyShaderModule(device, module, nullptr);
        };

        operator VkPipelineShaderStageCreateInfo() { return stage; };
        const VkPipelineShaderStageCreateInfo *operator&() const { return &stage; }

    private:
        VkPipelineShaderStageCreateInfo stage = {};
        VkShaderModule module = VK_NULL_HANDLE;

        VkDevice device = VK_NULL_HANDLE;
    };

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

    void Renderer::createModelPipeline()
    {
        ShaderStageGuard vertexShaderStage("shaders/modelVert.spv", VK_SHADER_STAGE_VERTEX_BIT, device);
        ShaderStageGuard fragmentShaderStage("shaders/modelFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, device);

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStage, fragmentShaderStage};

        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, tex);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, norm);

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &DEFAULT_BINDING_DESCRIPTION,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()};

        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.f};

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

        std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {modelPipeline.descriptorSetLayout, textures.descriptorSetLayout};

        VkPushConstantRange vertexPushConstantRange;
        vertexPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vertexPushConstantRange.offset = 0;
        vertexPushConstantRange.size = sizeof(VertexPushData);
        VkPushConstantRange materialPushConstantRange;
        materialPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        materialPushConstantRange.offset = materialPushDataStructOffset;
        materialPushConstantRange.size = sizeof(MaterialPushData);

        std::array<VkPushConstantRange, 2> pushConstantRanges = {vertexPushConstantRange, materialPushConstantRange};

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = pushConstantRanges.size(),
            .pPushConstantRanges = pushConstantRanges.data()};

        vkCheck(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &modelPipeline.layout), {'R', 210});

        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE};

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &HDR_COLOR_FORMAT;
        pipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputCreateInfo,
            .pInputAssemblyState = &DEFAULT_INPUT_ASSEMBLY_CREATE_INFO,
            .pViewportState = &DEFAULT_VIEWPORT_STATE_CREATE_INFO,
            .pRasterizationState = &rasterizerCreateInfo,
            .pMultisampleState = &DEFAULT_MULTISAMPLE_CREATE_INFO,
            .pDepthStencilState = &depthStencilCreateInfo,
            .pColorBlendState = &colorBlendingCreateInfo,
            .pDynamicState = &DEFAULT_DYNAMIC_STATE_CREATE_INFO,
            .layout = modelPipeline.layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1};

        vkCheck(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &modelPipeline.pipeline), {'R', 211});
    }

    void Renderer::createTransparentPipeline()
    {
        ShaderStageGuard vertexShaderStage("shaders/modelVert.spv", VK_SHADER_STAGE_VERTEX_BIT, device);
        ShaderStageGuard fragmentShaderStage("shaders/modelFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, device);

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStage, fragmentShaderStage};

        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, tex);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, norm);

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &DEFAULT_BINDING_DESCRIPTION,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()};

        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.f};

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

        std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {modelPipeline.descriptorSetLayout, textures.descriptorSetLayout};

        VkPushConstantRange vertexPushConstantRange;
        vertexPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vertexPushConstantRange.offset = 0;
        vertexPushConstantRange.size = sizeof(VertexPushData);
        VkPushConstantRange materialPushConstantRange;
        materialPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        materialPushConstantRange.offset = materialPushDataStructOffset;
        materialPushConstantRange.size = sizeof(MaterialPushData);

        std::array<VkPushConstantRange, 2> pushConstantRanges = {vertexPushConstantRange, materialPushConstantRange};

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = pushConstantRanges.size(),
            .pPushConstantRanges = pushConstantRanges.data()};

        vkCheck(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &transparentPipeline.layout), {'R', 210});

        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_FALSE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE};

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &HDR_COLOR_FORMAT;
        pipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputCreateInfo,
            .pInputAssemblyState = &DEFAULT_INPUT_ASSEMBLY_CREATE_INFO,
            .pViewportState = &DEFAULT_VIEWPORT_STATE_CREATE_INFO,
            .pRasterizationState = &rasterizerCreateInfo,
            .pMultisampleState = &DEFAULT_MULTISAMPLE_CREATE_INFO,
            .pDepthStencilState = &depthStencilCreateInfo,
            .pColorBlendState = &colorBlendingCreateInfo,
            .pDynamicState = &DEFAULT_DYNAMIC_STATE_CREATE_INFO,
            .layout = transparentPipeline.layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1};

        vkCheck(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &transparentPipeline.pipeline), {'R', 211});
    }

    void Renderer::createShadowPipeline()
    {
        ShaderStageGuard vertexShaderStage("shaders/shadowVert.spv", VK_SHADER_STAGE_VERTEX_BIT, device);

        VkVertexInputAttributeDescription attribute = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)};

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &DEFAULT_BINDING_DESCRIPTION,
            .vertexAttributeDescriptionCount = 1,
            .pVertexAttributeDescriptions = &attribute};

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_FRONT_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_TRUE,
            .depthBiasConstantFactor = 1.25f,
            .depthBiasSlopeFactor = 1.75f,
            .lineWidth = 1.f};

        VkViewport viewport = {
            0.f, 0.f,
            static_cast<float>(SHADOW_MAP_EXTENT.width),
            static_cast<float>(SHADOW_MAP_EXTENT.height),
            0.f, 1.f};

        VkRect2D scissor = {
            {0, 0},
            SHADOW_MAP_EXTENT};

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor};

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
        vkCheck(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &shadowPipeline.layout), {'R', 210});

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = 0;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = nullptr;
        pipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = 1,
            .pStages = &vertexShaderStage,
            .pVertexInputState = &vertexInputStateCreateInfo,
            .pInputAssemblyState = &DEFAULT_INPUT_ASSEMBLY_CREATE_INFO,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizationStateCreateInfo,
            .pMultisampleState = &DEFAULT_MULTISAMPLE_CREATE_INFO,
            .pDepthStencilState = &depthStencilStateCreateInfo,
            .pColorBlendState = &colorBlendStateCreateInfo,
            .layout = shadowPipeline.layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0};
        vkCheck(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &shadowPipeline.pipeline), {'R', 211});
    }

    void Renderer::createPostPipeline()
    {
        ShaderStageGuard vertexShaderStage("shaders/postVert.spv", VK_SHADER_STAGE_VERTEX_BIT, device);
        ShaderStageGuard fragmentShaderStage("shaders/postFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, device);

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStage, fragmentShaderStage};

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo blendState{};
        blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendState.attachmentCount = 1;
        blendState.pAttachments = &blendAttachment;

        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PostPushData);
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &postPipeline.descriptorSetLayout;
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &pushConstantRange;

        vkCheck(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &postPipeline.layout), {'R', 247});

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &swapChain.imageFormat;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &pipelineRenderingCreateInfo;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &DEFAULT_INPUT_ASSEMBLY_CREATE_INFO;
        pipelineInfo.pViewportState = &DEFAULT_VIEWPORT_STATE_CREATE_INFO;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &DEFAULT_MULTISAMPLE_CREATE_INFO;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &blendState;
        pipelineInfo.pDynamicState = &DEFAULT_DYNAMIC_STATE_CREATE_INFO;
        pipelineInfo.layout = postPipeline.layout;

        vkCheck(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &postPipeline.pipeline), {'R', 248});
    }

    void Renderer::createUIPipeline()
    {
        ShaderStageGuard vertexShaderStage("shaders/uiVert.spv", VK_SHADER_STAGE_VERTEX_BIT, device);
        ShaderStageGuard fragmentShaderStage("shaders/uiFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, device);

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStage, fragmentShaderStage};

        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, tex);

        VkPipelineVertexInputStateCreateInfo vertexInputState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &DEFAULT_BINDING_DESCRIPTION,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()};

        VkPipelineRasterizationStateCreateInfo rasterizationState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.f};

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

        VkPipelineColorBlendStateCreateInfo colorBlendState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState};

        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(UIPushData);

        std::array<VkDescriptorSetLayout, 2> uiSetLayouts = {uiPipeline.descriptorSetLayout, textures.descriptorSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayout = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(uiSetLayouts.size()),
            .pSetLayouts = uiSetLayouts.data(),
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange};

        vkCheck(vkCreatePipelineLayout(device, &pipelineLayout, nullptr, &uiPipeline.layout), {'R', 210});

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &swapChain.imageFormat;

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &DEFAULT_INPUT_ASSEMBLY_CREATE_INFO,
            .pViewportState = &DEFAULT_VIEWPORT_STATE_CREATE_INFO,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &DEFAULT_MULTISAMPLE_CREATE_INFO,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &DEFAULT_DYNAMIC_STATE_CREATE_INFO,
            .layout = uiPipeline.layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1};

        vkCheck(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &uiPipeline.pipeline), {'R', 211});
    }
}