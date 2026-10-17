// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

namespace VE
{
    void Renderer::recordShadowPass(const std::vector<ModelInstance> &modelInstances, const std::array<glm::mat4, MAX_SHADOW_MAPS> &lightSpaceMats)
    {
        if (shadowMapCount <= 1)
            return;

        for (size_t i = 1; i < shadowMapCount; i++)
        {
            VkImageMemoryBarrier2 imageMemoryBarrier = IMAGE_MEMORY_BARRIER_TEMPLATE;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            imageMemoryBarrier.image = shadowMaps[i].depthAttachment.image;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;

            VkDependencyInfo dependencyInfo{};
            dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

            vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

            VkRenderingAttachmentInfo shadowDepthAttachmentInfo{};
            shadowDepthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            shadowDepthAttachmentInfo.imageView = shadowMaps[i].depthAttachment.imageView;
            shadowDepthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            shadowDepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            shadowDepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            shadowDepthAttachmentInfo.clearValue.depthStencil = {1.f, 0};

            VkRenderingInfo shadowRenderingInfo{};
            shadowRenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
            shadowRenderingInfo.renderArea.offset = {0, 0};
            shadowRenderingInfo.renderArea.extent = SHADOW_MAP_EXTENT;
            shadowRenderingInfo.layerCount = 1;
            shadowRenderingInfo.pDepthAttachment = &shadowDepthAttachmentInfo;

            vkCmdBeginRendering(commandBuffer, &shadowRenderingInfo);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.pipeline);

            for (const ModelInstance &instance : modelInstances)
            {
                if (instance.lightIntensity > 0.f)
                    continue;

                for (const ModelBuffer &modelBuffer : modelBuffers)
                {
                    if (instance.modelHandle == modelBuffer.handle)
                    {
                        for (const MeshBuffer &meshBuffer : modelBuffer.meshBuffers)
                        {
                            if (meshBuffer.isTransparent)
                                continue;

                            VkBuffer vertexBuffers[] = {meshBuffer.vertexBuffer};
                            VkDeviceSize offsets[] = {0};
                            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                            vkCmdBindIndexBuffer(commandBuffer, meshBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                            ShadowPushData pushData{};
                            pushData.model = instance.modelMat;
                            pushData.lightSpaceMat = lightSpaceMats[i];

                            vkCmdPushConstants(commandBuffer, shadowPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowPushData), &pushData);

                            vkCmdDrawIndexed(commandBuffer, meshBuffer.indexCount, 1, 0, 0, 0);
                        }
                        break;
                    }
                }
            }

            vkCmdEndRendering(commandBuffer);

            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
            imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
            imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

            vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
        }
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

    ShadowMap Renderer::createShadowMap(ModelInstanceHandle instanceHandle)
    {
        return ShadowMap{
            .instanceHandle = instanceHandle,
            .depthAttachment = createAttachment(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, SHADOW_MAP_EXTENT)};
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

    void Renderer::writeShadowDescriptors()
    {
        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            std::array<VkDescriptorImageInfo, MAX_SHADOW_MAPS> shadowImageInfos{};
            for (size_t shadowIndex = 0; shadowIndex < MAX_SHADOW_MAPS; shadowIndex++)
            {
                const size_t srcIndex = shadowIndex < shadowMapCount ? shadowIndex : 0;
                shadowImageInfos[shadowIndex] = {
                    .sampler = shadowSampler,
                    .imageView = shadowMaps[srcIndex].depthAttachment.imageView,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
            }

            VkWriteDescriptorSet shadowSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = modelPipeline.descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(shadowImageInfos.size()),
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = shadowImageInfos.data()};

            vkUpdateDescriptorSets(device, 1, &shadowSetWrite, 0, nullptr);
        }
    }

    bool Renderer::syncShadowMaps(const std::vector<ModelInstance> &instances)
    {
        bool hasChanged = false;

        for (size_t i = 1; i < shadowMapCount; i++)
        {
            bool hasInstance = false;
            for (const ModelInstance &instance : instances)
            {
                if (shadowMaps[i].instanceHandle == instance.handle)
                {
                    hasInstance = true;
                    break;
                }
            }

            if (!hasInstance)
            {
                vkDeviceWaitIdle(device);

                destroyImageAttachment(shadowMaps[i].depthAttachment);

                shadowMapCount--;

                shadowMaps[i] = shadowMaps[shadowMapCount];

                i--;

                hasChanged = true;
            }
        }

        for (const ModelInstance &instance : instances)
        {
            if (instance.lightIntensity > 0.f)
            {
                bool hasShadowMap = false;
                for (size_t i = 1; i < shadowMapCount; i++)
                {
                    if (shadowMaps[i].instanceHandle == instance.handle)
                    {
                        hasShadowMap = true;
                        break;
                    }
                }

                if (!hasShadowMap)
                {
                    if (shadowMapCount >= MAX_SHADOW_MAPS)
                        Log::add('R', 228);

                    shadowMaps[shadowMapCount] = createShadowMap(instance.handle);
                    shadowMapCount++;

                    hasChanged = true;
                }
            }
        }

        return hasChanged;
    }
}