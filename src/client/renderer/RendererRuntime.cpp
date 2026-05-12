// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

#include <GLFW/glfw3.h>
#include <array>

namespace VE
{
    void Renderer::recordCommands(uint32_t currentImage, const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, color_t backgroundColor)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

        vkCheck(vkBeginCommandBuffer(commandBuffers[currentFrame], &commandBufferBeginInfo), {'V', 213});

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = swapChainFramebuffers[currentImage];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f};
        clearValues[1].depthStencil.depth = 1.0f;
        renderPassBeginInfo.pClearValues = clearValues.data();
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChainExtent.width),
            .height = static_cast<float>(swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f};
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChainExtent};
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

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
                        vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

                        vkCmdBindIndexBuffer(commandBuffers[currentFrame], meshBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                        PushData pushData;
                        pushData.model = instance.modelMat;
                        pushData.textureIndex = meshBuffer.texIndex;
                        pushData.lightStrength = instance.lightStrength;

                        vkCmdPushConstants(commandBuffers[currentFrame], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushData), &pushData);

                        std::array<VkDescriptorSet, 2> descriptorSetGroup = {descriptorSets[currentFrame], samplerDescriptorSets[meshBuffer.texIndex]};

                        vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);

                        vkCmdDrawIndexed(commandBuffers[currentFrame], meshBuffer.indexCount, 1, 0, 0, 0);
                    }

                    break;
                }
            }
        }

        vkCmdEndRenderPass(commandBuffers[currentFrame]);

        vkCheck(vkEndCommandBuffer(commandBuffers[currentFrame]), {'V', 213});
    }

    void Renderer::drawFrame(const DrawData &drawData, const glm::mat4 projectionMat)
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

        if (drawData.modelRemovedThisFrame)
        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            removeOrphanedModel(drawData.modelInstances);
        }

        glm::vec4 lightPos(0.0f);
        glm::vec3 lightColor(1.0f);
        for (const ModelInstance &instance : drawData.modelInstances)
        {
            for (const Model &model : drawData.models)
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

        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            syncModelBuffers(drawData.models);
            recordCommands(imageIndex, drawData.models, drawData.modelInstances, drawData.backgroundColor);
        }

        updateUniformBuffers(currentFrame, projectionMat, drawData.viewMat, lightPos, lightColor);

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &imageAvailableSemaphores[currentFrame],
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffers[currentFrame],
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

        currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
    }

    void Renderer::updateUniformBuffers(uint32_t imageIndex, glm::mat4 projectionMat, glm::mat4 viewMat, glm::vec4 lightPos, glm::vec3 lightColor)
    {
        UboCamera uboCamera;
        uboCamera.projection = projectionMat;
        uboCamera.view = viewMat;

        UboLighting uboLighting;
        uboLighting.lightPos = lightPos;
        uboLighting.lightColor = lightColor;
        uboLighting.viewPos = glm::inverse(viewMat)[3];

        void *cameraData;
        vkCheck(vkMapMemory(device, cameraUniformBufferMemory[imageIndex], 0, sizeof(UboCamera), 0, &cameraData), {'V', 236});
        memcpy(cameraData, &uboCamera, sizeof(UboCamera));
        vkUnmapMemory(device, cameraUniformBufferMemory[imageIndex]);

        void *lightingData;
        vkCheck(vkMapMemory(device, lightingUniformBufferMemory[imageIndex], 0, sizeof(UboLighting), 0, &lightingData), {'V', 236});
        memcpy(lightingData, &uboLighting, sizeof(UboLighting));
        vkUnmapMemory(device, lightingUniformBufferMemory[imageIndex]);
    }

    void Renderer::removeOrphanedModel(const std::vector<ModelInstance> &modelInstances)
    {
        for (std::vector<ModelBuffer>::iterator it = modelBuffers.begin(); it != modelBuffers.end();)
        {
            bool hasInstance = false;
            for (const ModelInstance &instance : modelInstances)
            {
                if (instance.modelHandle == it->handle)
                {
                    hasInstance = true;
                    break;
                }
            }

            if (!hasInstance)
            {
                for (MeshBuffer &meshBuffer : it->meshBuffers)
                    destroyMeshBuffer(meshBuffer);

                it = modelBuffers.erase(it);
            }
            else
            {
                ++it;
            }
        }
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
            for (VkFramebuffer framebuffer : swapChainFramebuffers)
            {
                if (framebuffer)
                    vkDestroyFramebuffer(device, framebuffer, nullptr);
            }

            if (depthBufferImageView)
                vkDestroyImageView(device, depthBufferImageView, nullptr);
            if (depthBufferImage)
                vkDestroyImage(device, depthBufferImage, nullptr);
            if (depthBufferImageMemory)
                vkFreeMemory(device, depthBufferImageMemory, nullptr);

            for (VkImageView imageView : swapChainImageViews)
            {
                if (imageView)
                    vkDestroyImageView(device, imageView, nullptr);
            }

            vkDestroySwapchainKHR(device, swapChain, nullptr);
        }

        Size2 newSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        createSwapChain(newSize);
        createImageViews();
        createDepthBufferImage();
        createFramebuffers();
    }
}