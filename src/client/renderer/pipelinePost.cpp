// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

namespace VE
{
    void Renderer::recordPostPass(uint32_t currentImage, const PostEffects &postEffects)
    {
        VkImageMemoryBarrier2 imageMemoryBarrier = IMAGE_MEMORY_BARRIER_TEMPLATE;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.image = swapChain.images[currentImage];
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = swapChain.imageViews[currentImage];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = swapChain.extent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, postPipeline.pipeline);

        VkViewport viewport = {
            .x = 0.f,
            .y = 0.f,
            .width = static_cast<float>(swapChain.extent.width),
            .height = static_cast<float>(swapChain.extent.height),
            .minDepth = 0.f,
            .maxDepth = 1.f};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChain.extent};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        PostPushData pushData;
        pushData.vignetteStrength = postEffects.vignetteStrength;
        pushData.vignetteRadius = postEffects.vignetteRadius;
        pushData.exposure = postEffects.exposure;

        pushData.flags = 0u;
        pushData.flags |= postEffects.fxaa ? POST_EFFECT_FXAA_BIT : 0u;
        pushData.flags |= postEffects.dithering ? POST_EFFECT_DITHERING_BIT : 0u;

        vkCmdPushConstants(commandBuffer, postPipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PostPushData), &pushData);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, postPipeline.layout, 0, 1, &postPipeline.descriptorSets[currentImage], 0, nullptr);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRendering(commandBuffer);
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
        samplerCreateInfo.mipLodBias = 0.f;
        samplerCreateInfo.minLod = 0.f;
        samplerCreateInfo.maxLod = 0.f;

        vkCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &postSampler), {'R', 226});
    }

    void Renderer::createPostDescriptors()
    {
        // Layout
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

        vkCheck(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &postPipeline.descriptorSetLayout), {'R', 217});

        // Pool
        VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = static_cast<uint32_t>(swapChain.imageCount)};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(swapChain.imageCount),
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &postPipeline.descriptorPool), {'R', 219});

        // Sets
        std::vector<VkDescriptorSetLayout> layouts(swapChain.imageCount, postPipeline.descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = postPipeline.descriptorPool;
        allocateInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.imageCount);
        allocateInfo.pSetLayouts = layouts.data();

        postPipeline.descriptorSets.resize(swapChain.imageCount);
        vkCheck(vkAllocateDescriptorSets(device, &allocateInfo, postPipeline.descriptorSets.data()), {'R', 220});

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
}