// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

namespace VE
{
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
}