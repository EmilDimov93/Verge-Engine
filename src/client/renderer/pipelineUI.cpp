// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include <cstring>

namespace VE
{
    void Renderer::updateUIUniformBuffers(uint32_t currentFrame)
    {
        UboUI uboUI;
        uboUI.orthographicProj = glm::ortho(0.f, (float)swapChain.extent.width, 0.f, (float)swapChain.extent.height);

        void *uiData;
        vkCheck(vkMapMemory(device, uiUniformBufferMemory, 0, sizeof(UboUI), 0, &uiData), {'R', 236});
        memcpy(uiData, &uboUI, sizeof(UboUI));
        vkUnmapMemory(device, uiUniformBufferMemory);
    }

    void Renderer::recordUIPass(uint32_t currentImage, const std::vector<Widget> &widgets, const std::vector<WidgetInstance> &widgetInstances)
    {
        VkImageMemoryBarrier2 imageMemoryBarrier = IMAGE_MEMORY_BARRIER_TEMPLATE;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.image = swapChain.images[currentImage];
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
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
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = swapChain.extent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = nullptr;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiPipeline.pipeline);

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

        for (const WidgetInstance &instance : widgetInstances)
        {
            for (const WidgetBuffer &widgetBuffer : widgetBuffers)
            {
                if (instance.widgetHandle == widgetBuffer.handle)
                {
                    for (const MeshBuffer &meshBuffer : widgetBuffer.meshBuffers)
                    {
                        VkBuffer vertexBuffers[] = {meshBuffer.vertexBuffer};
                        VkDeviceSize offsets[] = {0};
                        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

                        vkCmdBindIndexBuffer(commandBuffer, meshBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                        UIPushData pushData;
                        pushData.model = Transform(glm::vec3((instance.coords.x + 1) / 2 * swapChain.extent.width, (instance.coords.y + 1) / 2 * swapChain.extent.height, 0.f), glm::vec3(), glm::vec3(instance.uniformScale)).toMat();
                        pushData.model[1][1] *= -1;
                        pushData.textureIndex = widgetBuffer.materials[meshBuffer.materialIndex].texIndex;
                        pushData.color = widgetBuffer.materials[meshBuffer.materialIndex].baseColor;

                        vkCmdPushConstants(commandBuffer, uiPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UIPushData), &pushData);

                        std::array<VkDescriptorSet, 2> descriptorSetGroup = {uiPipeline.descriptorSets[currentFrame], textures.descriptorSets[widgetBuffer.materials[meshBuffer.materialIndex].texIndex]};

                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiPipeline.layout, 0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

                        vkCmdDrawIndexed(commandBuffer, meshBuffer.indexCount, 1, 0, 0, 0);
                    }

                    break;
                }
            }
        }

        vkCmdEndRendering(commandBuffer);

        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = 0;
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
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

    void Renderer::createUIUniformBuffers()
    {
        VkDeviceSize uiUniformBufferSize = sizeof(UboUI);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            createBuffer(uiUniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uiUniformBuffer.arr[i], &uiUniformBufferMemory.arr[i]);
    }

    void Renderer::createUIDescriptors()
    {
        // Layout
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

        vkCheck(vkCreateDescriptorSetLayout(device, &uiLayoutCreateInfo, nullptr, &uiPipeline.descriptorSetLayout), {'R', 217});

        // Pool
        VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(uiUniformBuffer.arr.size())};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = FRAMES_IN_FLIGHT,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &uiPipeline.descriptorPool), {'R', 219});

        // Sets
        uiPipeline.descriptorSets.resize(FRAMES_IN_FLIGHT);

        std::vector<VkDescriptorSetLayout> setLayouts(FRAMES_IN_FLIGHT, uiPipeline.descriptorSetLayout);

        VkDescriptorSetAllocateInfo setAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = uiPipeline.descriptorPool,
            .descriptorSetCount = FRAMES_IN_FLIGHT,
            .pSetLayouts = setLayouts.data()};

        vkCheck(vkAllocateDescriptorSets(device, &setAllocInfo, uiPipeline.descriptorSets.data()), {'R', 220});

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo uiBufferInfo = {
                .buffer = uiUniformBuffer.arr[i],
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
}