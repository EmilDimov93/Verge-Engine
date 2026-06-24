// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <algorithm>

namespace VE
{
    constexpr VkImageMemoryBarrier2 IMAGE_MEMORY_BARRIER_TEMPLATE = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange = {
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    void Renderer::recordShadowPass(const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, const glm::mat4 &lightSpaceMat)
    {
        const VkCommandBuffer commandBuffer = frames[currentFrame].commandBuffer;

        VkImageMemoryBarrier2 imageMemoryBarrier = IMAGE_MEMORY_BARRIER_TEMPLATE;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.image = shadowDepthAttachment.image;
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
            if(instance.lightIntensity > 0.f)
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
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
    }

    void Renderer::updateModelUniformBuffers(uint32_t currentFrame, glm::mat4 projectionMat, glm::mat4 viewMat, glm::vec4 lightPos, glm::vec3 lightColor, float lightIntensity, glm::mat4 lightSpaceMat, float outdoorBrightness)
    {
        UboCamera uboCamera;
        uboCamera.projection = projectionMat;
        uboCamera.view = viewMat;
        uboCamera.lightSpaceMat = lightSpaceMat;

        void *cameraData;
        vkCheck(vkMapMemory(device, cameraUniformBufferMemory[currentFrame], 0, sizeof(UboCamera), 0, &cameraData), {'R', 236});
        memcpy(cameraData, &uboCamera, sizeof(UboCamera));
        vkUnmapMemory(device, cameraUniformBufferMemory[currentFrame]);

        UboLighting uboLighting;
        uboLighting.lightPos = lightPos;
        uboLighting.lightColor = lightColor;
        uboLighting.lightIntensity = lightIntensity;
        uboLighting.viewPos = glm::inverse(viewMat)[3];
        uboLighting.outdoorBrightness = outdoorBrightness;

        void *lightingData;
        vkCheck(vkMapMemory(device, lightingUniformBufferMemory[currentFrame], 0, sizeof(UboLighting), 0, &lightingData), {'R', 236});
        memcpy(lightingData, &uboLighting, sizeof(UboLighting));
        vkUnmapMemory(device, lightingUniformBufferMemory[currentFrame]);
    }

    void Renderer::recordModelPass(uint32_t currentImage, const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, color_t backgroundColor, const glm::mat4 &lightSpaceMat, const glm::vec3 &cameraPosition)
    {
        const VkCommandBuffer commandBuffer = frames[currentFrame].commandBuffer;

        VkImageMemoryBarrier2 imageMemoryBarrier = IMAGE_MEMORY_BARRIER_TEMPLATE;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.image = prePostAttachments[currentImage].image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkImageMemoryBarrier2 depthMemoryBarrier = IMAGE_MEMORY_BARRIER_TEMPLATE;
        depthMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthMemoryBarrier.image = depthAttachments[currentFrame].image;
        depthMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthMemoryBarrier.srcAccessMask = 0;
        depthMemoryBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depthMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        depthMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;

        std::array<VkImageMemoryBarrier2, 2> barriers = {imageMemoryBarrier, depthMemoryBarrier};

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size());
        dependencyInfo.pImageMemoryBarriers = barriers.data();

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = prePostAttachments[currentImage].imageView;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = {{{backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a}}};

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.imageView = depthAttachments[currentFrame].imageView;
        depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentInfo.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = swapChain.extent;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachmentInfo;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline.pipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChain.extent.width),
            .height = static_cast<float>(swapChain.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChain.extent};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        auto drawMesh = [&](const ModelInstance &instance, const MeshBuffer &meshBuffer, Material material, VkPipelineLayout layout)
        {
            VkBuffer vertexBuffers[] = {meshBuffer.vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, meshBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            VertexPushData vertexPushData;
            vertexPushData.model = instance.modelMat;
            vertexPushData.lightIntensity = instance.lightIntensity;

            MaterialPushData materialPushData;
            materialPushData.baseColor = material.baseColor;
            materialPushData.metallic = material.metallic;
            materialPushData.roughness = material.roughness;
            materialPushData.textureIndex = material.texIndex;

            vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexPushData), &vertexPushData);

            vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, materialPushDataStructOffset, sizeof(MaterialPushData), &materialPushData);

            std::array<VkDescriptorSet, 2> descriptorSetGroup = {modelPipeline.descriptorSets[currentFrame], textures.descriptorSets[material.texIndex]};

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline.layout, 0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

            vkCmdDrawIndexed(commandBuffer, meshBuffer.indexCount, 1, 0, 0, 0);
        };

        struct TransparentMesh
        {
            const ModelInstance *instance;
            const MeshBuffer *meshBuffer;
            Material material;
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
                            transparentMeshes.push_back({&instance, &meshBuffer, modelBuffer.materials[meshBuffer.materialIndex], distanceSquared});
                            continue;
                        }

                        drawMesh(instance, meshBuffer, modelBuffer.materials[meshBuffer.materialIndex], modelPipeline.layout);
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
                drawMesh(*mesh.instance, *mesh.meshBuffer, mesh.material, transparentPipeline.layout);
        }

        vkCmdEndRendering(commandBuffer);

        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

        VkDependencyInfo endDependencyInfo{};
        endDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        endDependencyInfo.imageMemoryBarrierCount = 1;
        endDependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

        vkCmdPipelineBarrier2(commandBuffer, &endDependencyInfo);
    }

    void Renderer::recordPostPass(uint32_t currentImage, const PostEffects &postEffects)
    {
        const VkCommandBuffer commandBuffer = frames[currentFrame].commandBuffer;

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
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChain.extent.width),
            .height = static_cast<float>(swapChain.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChain.extent};
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
        uboUI.orthographicProj = glm::ortho(0.f, (float)swapChain.extent.width, 0.f, (float)swapChain.extent.height);

        void *uiData;
        vkCheck(vkMapMemory(device, uiUniformBuffersMemory[currentFrame], 0, sizeof(UboUI), 0, &uiData), {'R', 236});
        memcpy(uiData, &uboUI, sizeof(UboUI));
        vkUnmapMemory(device, uiUniformBuffersMemory[currentFrame]);
    }

    void Renderer::recordUIPass(uint32_t currentImage, const std::vector<Widget> &widgets, const std::vector<WidgetInstance> &widgetInstances)
    {
        const VkCommandBuffer commandBuffer = frames[currentFrame].commandBuffer;

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
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChain.extent.width),
            .height = static_cast<float>(swapChain.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f};
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
                        pushData.model = Transform(Position3((instance.coords.x + 1) / 2 * swapChain.extent.width, (instance.coords.y + 1) / 2 * swapChain.extent.height, 0.f), Rotation3(), Scale3(instance.uniformScale)).toMat();
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

    void Renderer::drawFrame(const SceneDrawData &sceneDrawData, const UIDrawData &uiDrawData, const glm::mat4 projectionMat, const PostEffects &postEffects)
    {
        vkCheck(vkWaitForFences(device, 1, &frames[currentFrame].drawFence, VK_TRUE, UINT64_MAX), {'R', 231});

        uint32_t imageIndex;
        VkResult imageResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, frames[currentFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (imageResult == VK_ERROR_OUT_OF_DATE_KHR)
            recreateSwapChain();
        else if (imageResult != VK_SUCCESS && imageResult != VK_SUBOPTIMAL_KHR)
            Log::add('R', 230);

        vkCheck(vkResetFences(device, 1, &frames[currentFrame].drawFence), {'R', 232});

        if (sceneDrawData.modelRemovedThisFrame)
        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            removeOrphanedModel(sceneDrawData.modelInstances);
        }

        glm::vec4 lightPos(0.0f);
        glm::vec3 lightColor(1.0f);
        float lightIntensity = 0.f;
        for (const ModelInstance &instance : sceneDrawData.modelInstances)
        {
            if (instance.lightIntensity > 0.f)
            {
                lightPos = instance.modelMat[3];
                lightColor = instance.lightColor;
                lightIntensity = instance.lightIntensity;
            }
        }

        glm::vec3 cameraPosition = glm::vec3(glm::inverse(sceneDrawData.viewMat)[3]);
        glm::vec3 cameraForward = -glm::vec3(glm::inverse(sceneDrawData.viewMat)[2]);
        glm::vec3 shadowCenter = cameraPosition + cameraForward * 35.f;
        glm::mat4 lightView = glm::lookAt(glm::vec3(lightPos), shadowCenter, glm::vec3(0, 1, 0));

        glm::mat4 lightProjection = glm::orthoZO(-50.f, 50.f, -50.f, 50.f, 1.f, 200.f);
        glm::mat4 lightSpaceMat = lightProjection * lightView;

        updateModelUniformBuffers(currentFrame, projectionMat, sceneDrawData.viewMat, lightPos, lightColor, lightIntensity, lightSpaceMat, sceneDrawData.outdoorBrightness);
        updateUIUniformBuffers(currentFrame);

        const VkCommandBuffer commandBuffer = frames[currentFrame].commandBuffer;

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkCheck(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), {'R', 213});

        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            syncModelBuffers(sceneDrawData.models);

            recordShadowPass(sceneDrawData.models, sceneDrawData.modelInstances, lightSpaceMat);

            recordModelPass(imageIndex, sceneDrawData.models, sceneDrawData.modelInstances, sceneDrawData.backgroundColor, lightSpaceMat, glm::vec3(glm::inverse(sceneDrawData.viewMat)[3]));
        }

        recordPostPass(imageIndex, postEffects);

        {
            std::lock_guard<std::recursive_mutex> lock(widgetMutex);
            syncWidgetBuffers(uiDrawData.widgets);

            recordUIPass(imageIndex, uiDrawData.widgets, uiDrawData.widgetInstances);
        }

        vkCheck(vkEndCommandBuffer(commandBuffer), {'R', 213});

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frames[currentFrame].imageAvailableSemaphore,
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderFinishedSemaphores[imageIndex]};

        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            vkCheck(vkQueueSubmit(graphicsQueue, 1, &submitInfo, frames[currentFrame].drawFence), {'R', 233});
        }

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphores[imageIndex],
            .swapchainCount = 1,
            .pSwapchains = &swapChain.swapChain,
            .pImageIndices = &imageIndex};

        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            VkResult presentResult = vkQueuePresentKHR(presentQueue, &presentInfo);

            if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
                recreateSwapChain();
            else if (presentResult != VK_SUCCESS)
                Log::add('R', 234);
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

        for (ImageAttachment &depthAttachment : depthAttachments)
            destroyImageAttachment(depthAttachment);

        destroySwapChain(swapChain);

        for (ImageAttachment &attachment : prePostAttachments)
            destroyImageAttachment(attachment);

        createSwapChain(Size2(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
        createPrePostImages();
        updatePostDescriptorSets();
        createDepthAttachments();
    }
}