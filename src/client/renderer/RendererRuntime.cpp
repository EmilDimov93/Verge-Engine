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

    void Renderer::recordShadowPass(const std::vector<ModelInstance> &modelInstances, const std::array<glm::mat4, MAX_SHADOW_MAPS> &lightSpaceMats)
    {
        if (shadowMapCount <= 1)
            return;

        const VkCommandBuffer commandBuffer = frames[currentFrame].commandBuffer;

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
            shadowRenderingInfo.renderArea.extent = {SHADOW_MAP_EXTENT.x, SHADOW_MAP_EXTENT.y};
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

    void Renderer::updateModelUniformBuffers(uint32_t currentFrame, const glm::mat4 &projectionMat, const glm::mat4 &viewMat, const std::array<glm::vec4, MAX_SHADOW_MAPS> &lightPositions, const std::array<glm::vec4, MAX_SHADOW_MAPS> &lightColors, const std::array<glm::mat4, MAX_SHADOW_MAPS> &lightSpaceMats, int32_t lightCount, float outdoorBrightness)
    {
        UboCamera uboCamera;
        uboCamera.projection = projectionMat;
        uboCamera.view = viewMat;

        void *cameraData;
        vkCheck(vkMapMemory(device, cameraUniformBufferMemory[currentFrame], 0, sizeof(UboCamera), 0, &cameraData), {'R', 236});
        memcpy(cameraData, &uboCamera, sizeof(UboCamera));
        vkUnmapMemory(device, cameraUniformBufferMemory[currentFrame]);

        UboLighting uboLighting;
        std::memcpy(uboLighting.lightPositions, lightPositions.data(), sizeof(uboLighting.lightPositions));
        std::memcpy(uboLighting.lightColors, lightColors.data(), sizeof(uboLighting.lightColors));
        std::memcpy(uboLighting.lightSpaceMats, lightSpaceMats.data(), sizeof(uboLighting.lightSpaceMats));
        uboLighting.viewPos = glm::inverse(viewMat)[3];
        uboLighting.lightCount = lightCount;
        uboLighting.outdoorBrightness = outdoorBrightness;

        void *lightingData;
        vkCheck(vkMapMemory(device, lightingUniformBufferMemory[currentFrame], 0, sizeof(UboLighting), 0, &lightingData), {'R', 236});
        memcpy(lightingData, &uboLighting, sizeof(UboLighting));
        vkUnmapMemory(device, lightingUniformBufferMemory[currentFrame]);
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

    void Renderer::recordModelPass(uint32_t currentImage, const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, color_t backgroundColor, const glm::vec3 &cameraPosition)
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
        depthMemoryBarrier.image = gBuffers[currentFrame].depth.image;
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
        depthAttachmentInfo.imageView = gBuffers[currentFrame].depth.imageView;
        depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentInfo.clearValue.depthStencil = {1.f, 0};

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

        auto drawMesh = [&](const ModelInstance &instance, const MeshBuffer &meshBuffer, const Material &material, VkPipelineLayout layout)
        {
            VkBuffer vertexBuffers[] = {meshBuffer.vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, meshBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            VertexPushData vertexPushData;
            vertexPushData.model = instance.modelMat;

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
            const Material *material;
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
                            transparentMeshes.push_back({&instance, &meshBuffer, &modelBuffer.materials[meshBuffer.materialIndex], distanceSquared});
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
                drawMesh(*mesh.instance, *mesh.meshBuffer, *mesh.material, transparentPipeline.layout);
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

    void Renderer::drawFrame(const SceneDrawData &sceneDrawData, const UIDrawData &uiDrawData, const glm::mat4 projectionMat, const PostEffects &postEffects)
    {
        vkCheck(vkWaitForFences(device, 1, &frames[currentFrame].drawFence, VK_TRUE, UINT64_MAX), {'R', 231});

        uint32_t imageIndex;
        VkResult imageResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, frames[currentFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (imageResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (imageResult != VK_SUCCESS && imageResult != VK_SUBOPTIMAL_KHR)
        {
            Log::add('R', 230);
        }

        vkCheck(vkResetFences(device, 1, &frames[currentFrame].drawFence), {'R', 232});

        if (sceneDrawData.modelRemovedThisFrame)
        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            removeOrphanedModel(sceneDrawData.modelInstances);
        }

        updateUIUniformBuffers(currentFrame);

        const VkCommandBuffer commandBuffer = frames[currentFrame].commandBuffer;

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkCheck(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), {'R', 213});

        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            syncModelBuffers(sceneDrawData.models);

            bool hasShadowMapsChanged = syncShadowMaps(sceneDrawData.modelInstances);
            if (hasShadowMapsChanged)
            {
                vkDeviceWaitIdle(device);
                writeShadowDescriptors();
            }

            std::array<glm::mat4, MAX_SHADOW_MAPS> lightSpaceMats{};
            std::array<glm::vec4, MAX_SHADOW_MAPS> lightPositions{};
            std::array<glm::vec4, MAX_SHADOW_MAPS> lightColors{};
            for (glm::mat4 &mat : lightSpaceMats)
                mat = glm::mat4(1.f);

            const glm::vec3 cameraPosition = glm::vec3(glm::inverse(sceneDrawData.viewMat)[3]);
            const glm::vec3 cameraForward = -glm::vec3(glm::inverse(sceneDrawData.viewMat)[2]);
            const glm::vec3 shadowCenter = cameraPosition + cameraForward * 35.f;
            const glm::mat4 lightProjection = glm::orthoZO(-50.f, 50.f, -50.f, 50.f, 1.f, 200.f);

            for (const ModelInstance &instance : sceneDrawData.modelInstances)
            {
                if (instance.lightIntensity <= 0.f)
                    continue;

                size_t slot = 0;
                for (size_t i = 1; i < shadowMapCount; i++)
                    if (shadowMaps[i].instanceHandle == instance.handle)
                    {
                        slot = i;
                        break;
                    }
                if (slot == 0)
                    continue;

                const glm::vec3 lightPosition = glm::vec3(instance.modelMat[3]);
                const glm::mat4 lightView = glm::lookAt(lightPosition, shadowCenter, glm::vec3(0, 1, 0));
                lightSpaceMats[slot] = lightProjection * lightView;
                lightPositions[slot] = glm::vec4(lightPosition, 0.f); // lightPosition.w is unused
                lightColors[slot] = instance.lightColor;
                lightColors[slot].a = instance.lightIntensity; // lightColor.a is intensity
            }

            updateModelUniformBuffers(currentFrame, projectionMat, sceneDrawData.viewMat, lightPositions, lightColors, lightSpaceMats, static_cast<int32_t>(shadowMapCount), sceneDrawData.outdoorBrightness);

            recordShadowPass(sceneDrawData.modelInstances, lightSpaceMats);

            recordModelPass(imageIndex, sceneDrawData.models, sceneDrawData.modelInstances, sceneDrawData.backgroundColor, glm::vec3(glm::inverse(sceneDrawData.viewMat)[3]));
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

        for (GeometryBuffer &gBuffers : gBuffers)
        {
            destroyImageAttachment(gBuffers.depth);
            destroyImageAttachment(gBuffers.normal);
            destroyImageAttachment(gBuffers.metallic);
            destroyImageAttachment(gBuffers.roughness);
        }

        destroySwapChain(swapChain);

        for (ImageAttachment &attachment : prePostAttachments)
            destroyImageAttachment(attachment);

        createSwapChain(glm::uvec2(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
        createPrePostImages();
        updatePostDescriptorSets();
        createGeometryBuffers();
    }
}