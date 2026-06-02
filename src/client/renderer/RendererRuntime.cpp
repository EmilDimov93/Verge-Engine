// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <algorithm>

namespace VE
{
    void Renderer::recordShadowPass(const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, const glm::mat4 &lightSpaceMat)
    {
        const VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = shadowDepthAttachment.image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        VkRenderingAttachmentInfo shadowDepthAttachmentInfo{};
        shadowDepthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        shadowDepthAttachmentInfo.imageView = shadowDepthAttachment.imageView;
        shadowDepthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowDepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadowDepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        shadowDepthAttachmentInfo.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo shadowRenderingInfo{};
        shadowRenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        shadowRenderingInfo.renderArea.offset = {0, 0};
        shadowRenderingInfo.renderArea.extent = {SHADOW_MAP_EXTENT.w, SHADOW_MAP_EXTENT.h};
        shadowRenderingInfo.layerCount = 1;
        shadowRenderingInfo.pDepthAttachment = &shadowDepthAttachmentInfo;

        vkCmdBeginRendering(commandBuffer, &shadowRenderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.pipeline);

        for (const ModelInstance &instance : modelInstances)
        {
            for (const ModelBuffer &modelBuffer : modelBuffers)
            {
                if (instance.modelHandle == modelBuffer.handle)
                {
                    for (const MeshBuffer &meshBuffer : modelBuffer.meshBuffers)
                    {
                        VkBuffer vertexBuffers[] = {meshBuffer.vertexBuffer};
                        VkDeviceSize offsets[] = {0};
                        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                        vkCmdBindIndexBuffer(commandBuffer, meshBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                        ShadowPushData pushData{};
                        pushData.model = instance.modelMat;
                        pushData.lightSpaceMat = lightSpaceMat;

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
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }

    void Renderer::updateModelUniformBuffers(uint32_t currentFrame, glm::mat4 projectionMat, glm::mat4 viewMat, glm::vec4 lightPos, glm::vec3 lightColor, glm::mat4 lightSpaceMat)
    {
        UboCamera uboCamera;
        uboCamera.projection = projectionMat;
        uboCamera.view = viewMat;
        uboCamera.lightSpaceMat = lightSpaceMat;

        void *cameraData;
        vkCheck(vkMapMemory(device, cameraUniformBufferMemory[currentFrame], 0, sizeof(UboCamera), 0, &cameraData), {'V', 236});
        memcpy(cameraData, &uboCamera, sizeof(UboCamera));
        vkUnmapMemory(device, cameraUniformBufferMemory[currentFrame]);

        UboLighting uboLighting;
        uboLighting.lightPos = lightPos;
        uboLighting.lightColor = lightColor;
        uboLighting.viewPos = glm::inverse(viewMat)[3];

        void *lightingData;
        vkCheck(vkMapMemory(device, lightingUniformBufferMemory[currentFrame], 0, sizeof(UboLighting), 0, &lightingData), {'V', 236});
        memcpy(lightingData, &uboLighting, sizeof(UboLighting));
        vkUnmapMemory(device, lightingUniformBufferMemory[currentFrame]);
    }

    void Renderer::recordMainPass(uint32_t currentImage, const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, color_t backgroundColor, const glm::mat4 &lightSpaceMat, const glm::vec3 &cameraPosition)
    {
        const VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = prePostAttachments[currentImage].image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = prePostAttachments[currentImage].imageView;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = {{{backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a}}};

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.imageView = depthAttachment.imageView;
        depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentInfo.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = swapChainExtent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachmentInfo;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline.pipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChainExtent.width),
            .height = static_cast<float>(swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChainExtent};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        auto drawMesh = [&](const ModelInstance &instance, const MeshBuffer &meshBuffer, VkPipelineLayout pipelineLayout)
        {
            VkBuffer vertexBuffers[] = {meshBuffer.vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, meshBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            PushData pushData;
            pushData.model = instance.modelMat;
            pushData.textureIndex = meshBuffer.texIndex;
            pushData.lightStrength = instance.lightStrength;

            vkCmdPushConstants(commandBuffer, modelPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushData), &pushData);

            std::array<VkDescriptorSet, 2> descriptorSetGroup = {modelPipeline.descriptorSets[currentFrame], textures.descriptorSets[meshBuffer.texIndex]};

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline.layout, 0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

            vkCmdDrawIndexed(commandBuffer, meshBuffer.indexCount, 1, 0, 0, 0);
        };

        struct TransparentMesh
        {
            const ModelInstance *instance;
            const MeshBuffer *meshBuffer;
            float distanceSquared;
        };
        std::vector<TransparentMesh> transparentMeshes;

        for (const ModelInstance &instance : modelInstances)
        {
            for (const ModelBuffer &modelBuffer : modelBuffers)
            {
                if (instance.modelHandle == modelBuffer.handle)
                {
                    for (const MeshBuffer &meshBuffer : modelBuffer.meshBuffers)
                    {
                        if (meshBuffer.isTransparent)
                        {
                            glm::vec3 meshWorldPosition = glm::vec3(instance.modelMat[3]);
                            float distanceSquared = glm::dot(meshWorldPosition - cameraPosition, meshWorldPosition - cameraPosition);
                            transparentMeshes.push_back({&instance, &meshBuffer, distanceSquared});
                            continue;
                        }

                        drawMesh(instance, meshBuffer, modelPipeline.layout);
                    }

                    break;
                }
            }
        }

        if (!transparentMeshes.empty())
        {
            std::sort(transparentMeshes.begin(), transparentMeshes.end(),
                [](const TransparentMesh &leftMesh, const TransparentMesh &rightMesh)
                { return leftMesh.distanceSquared > rightMesh.distanceSquared; });

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, transparentPipeline.pipeline);

            for (const TransparentMesh &mesh : transparentMeshes)
                drawMesh(*mesh.instance, *mesh.meshBuffer, transparentPipeline.layout);
        }

        vkCmdEndRendering(commandBuffer);

        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }

    void Renderer::recordPostPass(uint32_t currentImage, const PostEffects &postEffects)
    {
        const VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = swapChainImages[currentImage];
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = swapChainImageViews[currentImage];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = swapChainExtent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, postPipeline.pipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChainExtent.width),
            .height = static_cast<float>(swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChainExtent};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        PostPushData pushData;
        pushData.vignetteStrength = postEffects.vignetteStrength;
        pushData.vignetteRadius = postEffects.vignetteRadius;

        pushData.flags = 0u;
        pushData.flags |= postEffects.fxaa ? POST_EFFECT_FXAA_BIT : 0u;
        pushData.flags |= postEffects.dithering ? POST_EFFECT_DITHERING_BIT : 0u;

        vkCmdPushConstants(commandBuffer, postPipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PostPushData), &pushData);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, postPipeline.layout, 0, 1, &postPipeline.descriptorSets[currentImage], 0, nullptr);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRendering(commandBuffer);
    }

    void Renderer::updateUIUniformBuffers(uint32_t currentFrame)
    {
        UboUI uboUI;
        uboUI.orthographicProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);

        void *uiData;
        vkCheck(vkMapMemory(device, uiUniformBuffersMemory[currentFrame], 0, sizeof(UboUI), 0, &uiData), {'V', 236});
        memcpy(uiData, &uboUI, sizeof(UboUI));
        vkUnmapMemory(device, uiUniformBuffersMemory[currentFrame]);
    }

    void Renderer::recordUIPass(uint32_t currentImage, const std::vector<Widget> &widgets, const std::vector<WidgetInstance> &widgetInstances)
    {
        const VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = swapChainImages[currentImage];
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = swapChainImageViews[currentImage];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = swapChainExtent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = nullptr;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uiPipeline.pipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChainExtent.width),
            .height = static_cast<float>(swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChainExtent};
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
                        pushData.model = instance.modelMat;
                        const float aspect = (float)swapChainExtent.width / (float)swapChainExtent.height;
                        pushData.model = glm::scale(pushData.model, glm::vec3(1.0f / aspect, 1.0f, 1.0f));

                        pushData.textureIndex = meshBuffer.texIndex;
                        pushData.model[1][1] *= -1;

                        vkCmdPushConstants(commandBuffer, uiPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UIPushData), &pushData);

                        std::array<VkDescriptorSet, 2> descriptorSetGroup = {uiPipeline.descriptorSets[currentFrame], textures.descriptorSets[meshBuffer.texIndex]};

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
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = 0;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    }

    void Renderer::drawFrame(const SceneDrawData &sceneDrawData, const UIDrawData &uiDrawData, const glm::mat4 projectionMat, const PostEffects &postEffects)
    {
        vkCheck(vkWaitForFences(device, 1, &drawFences[currentFrame], VK_TRUE, UINT64_MAX), {'V', 231});

        uint32_t imageIndex;
        VkResult imageResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (imageResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            framebufferResized = false;
            return;
        }
        else if (imageResult != VK_SUCCESS && imageResult != VK_SUBOPTIMAL_KHR)
        {
            Log::add('V', 230);
        }

        vkCheck(vkResetFences(device, 1, &drawFences[currentFrame]), {'V', 232});

        if (sceneDrawData.modelRemovedThisFrame)
        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            removeOrphanedModel(sceneDrawData.modelInstances);
        }

        glm::vec4 lightPos(0.0f);
        glm::vec3 lightColor(1.0f);
        for (const ModelInstance &instance : sceneDrawData.modelInstances)
        {
            for (const Model &model : sceneDrawData.models)
            {
                if (model.getHandle() == instance.modelHandle)
                {
                    lightPos = glm::vec4(glm::vec3(instance.modelMat[3]), instance.lightStrength);
                    lightColor = instance.lightColor;
                    break;
                }
            }
            if (lightPos.w > 0.0f)
                break;
        }

        glm::mat4 lightView = glm::lookAt(glm::vec3(lightPos), glm::vec3(0.0f), glm::vec3(0, 1, 0));
        glm::mat4 lightProjection = glm::orthoZO(-50.f, 50.f, -50.f, 50.f, 1.f, 200.f);
        glm::mat4 lightSpaceMat = lightProjection * lightView;

        updateModelUniformBuffers(currentFrame, projectionMat, sceneDrawData.viewMat, lightPos, lightColor, lightSpaceMat);
        updateUIUniformBuffers(currentFrame);

        const VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkCheck(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), {'V', 213});

        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            syncModelBuffers(sceneDrawData.models);

            recordShadowPass(sceneDrawData.models, sceneDrawData.modelInstances, lightSpaceMat);

            recordMainPass(imageIndex, sceneDrawData.models, sceneDrawData.modelInstances, sceneDrawData.backgroundColor, lightSpaceMat, glm::vec3(glm::inverse(sceneDrawData.viewMat)[3]));
        }

        recordPostPass(imageIndex, postEffects);

        {
            std::lock_guard<std::recursive_mutex> lock(widgetMutex);
            syncWidgetBuffers(uiDrawData.widgets);

            recordUIPass(imageIndex, uiDrawData.widgets, uiDrawData.widgetInstances);
        }

        vkCheck(vkEndCommandBuffer(commandBuffer), {'V', 213});

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &imageAvailableSemaphores[currentFrame],
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderFinishedSemaphores[imageIndex]};

        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            vkCheck(vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]), {'V', 233});
        }

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphores[imageIndex],
            .swapchainCount = 1,
            .pSwapchains = &swapChain,
            .pImageIndices = &imageIndex};

        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            VkResult presentResult = vkQueuePresentKHR(presentQueue, &presentInfo);

            if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized)
            {
                recreateSwapChain();
                framebufferResized = false;
            }
            else if (presentResult != VK_SUCCESS)
            {
                Log::add('V', 234);
            }
        }

        currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
    }

    void Renderer::recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        if (swapChain)
        {
            destroyImageAttachment(depthAttachment);

            for (VkImageView imageView : swapChainImageViews)
                if (imageView)
                    vkDestroyImageView(device, imageView, nullptr);

            vkDestroySwapchainKHR(device, swapChain, nullptr);
        }

        for (ImageAttachment &attachment : prePostAttachments)
            destroyImageAttachment(attachment);

        createSwapChain(Size2(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
        createPrePostImages();
        updatePostDescriptorSets();
        createDepthAttachment();
    }
}