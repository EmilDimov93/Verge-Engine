// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

namespace VE
{

    void Renderer::drawFrame(const SceneDrawData &sceneDrawData, const UIDrawData &uiDrawData, const glm::mat4 projectionMat, const PostEffects &postEffects)
    {
        vkCheck(vkWaitForFences(device, 1, drawFence.ptr(), VK_TRUE, UINT64_MAX), {'R', 231});

        uint32_t imageIndex;
        VkResult imageResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (imageResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (imageResult != VK_SUCCESS && imageResult != VK_SUBOPTIMAL_KHR)
        {
            Log::add('R', 230);
        }

        vkCheck(vkResetFences(device, 1, drawFence.ptr()), {'R', 232});

        if (sceneDrawData.modelRemovedThisFrame)
        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            removeOrphanedModel(sceneDrawData.modelInstances);
        }

        updateUIUniformBuffers(currentFrame);

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
            .pWaitSemaphores = imageAvailableSemaphore.ptr(),
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = commandBuffer.ptr(),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderFinishedSemaphores[imageIndex]};

        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            vkCheck(vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFence), {'R', 233});
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
}