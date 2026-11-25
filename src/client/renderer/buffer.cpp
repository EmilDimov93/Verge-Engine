// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

namespace VE
{
    void Renderer::createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory) const
    {
        std::array<uint32_t, 2> queueFamilyIndices = {graphicsQueueFamilyIndex, transferQueueFamilyIndex};
        VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = bufferUsageFlags,
            .sharingMode = VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = 2,
            .pQueueFamilyIndices = queueFamilyIndices.data()};

        vkCheck(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer), {'R', 218});

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

        VkMemoryAllocateInfo memoryAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, bufferPropertyFlags)};

        vkCheck(vkAllocateMemory(device, &memoryAllocInfo, nullptr, bufferMemory), {'R', 218});

        vkCheck(vkBindBufferMemory(device, *buffer, *bufferMemory, 0), {'R', 218});
    }

    VkResult Renderer::copyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkFence fence) const
    {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo), {'R', 213});

        VkBufferCopy bufferCopyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = bufferSize};

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

        vkCheck(vkEndCommandBuffer(commandBuffer), {'R', 213});

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer};

        vkCheck(vkResetFences(device, 1, &fence), {'R', 232});
        {
            std::lock_guard<std::mutex> lock(transferQueueMutex);
            vkCheck(vkQueueSubmit(transferQueue, 1, &submitInfo, fence), {'R', 233});
        }
        vkCheck(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), {'R', 231});

        return VK_SUCCESS;
    }
}