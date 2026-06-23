// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

#include <array>

namespace VE
{
    void Renderer::vkCheck(VkResult res, ErrorCode errorCode)
    {
        switch (res)
        {
        case VK_SUCCESS:
            return;
        case VK_NOT_READY:
            Log::add('R', 100);
            break;
        case VK_TIMEOUT:
            Log::add('R', 101);
            break;
        case VK_SUBOPTIMAL_KHR:
            Log::add('R', 102);
            break;
        case VK_EVENT_SET:
            Log::add('R', 103);
            break;
        case VK_EVENT_RESET:
            Log::add('R', 104);
            break;
        default:
            Log::add(errorCode.letter, errorCode.number);
        }
    }

    uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            if ((allowedTypes & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            {
                return i;
            }
        }

        Log::add('R', 237);
        return -1;
    }

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

    VkResult Renderer::copyBuffer(VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkFence fence) const
    {
        VkCommandBuffer transferCommandBuffer;

        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = transferCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};

        vkCheck(vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer), {'R', 212});

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        vkCheck(vkBeginCommandBuffer(transferCommandBuffer, &beginInfo), {'R', 213});

        VkBufferCopy bufferCopyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = bufferSize};

        vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

        vkCheck(vkEndCommandBuffer(transferCommandBuffer), {'R', 213});

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &transferCommandBuffer};

        {
            std::lock_guard<std::mutex> lock(transferQueueMutex);
            vkCheck(vkQueueSubmit(transferQueue, 1, &submitInfo, fence), {'R', 233});
        }

        vkCheck(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), {'R', 231});

        vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);

        return VK_SUCCESS;
    }

    VkImage Renderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, uint32_t mipLevelCount, VkDeviceMemory *imageMemory)
    {
        std::array<uint32_t, 2> queueFamilyIndices = {graphicsQueueFamilyIndex, transferQueueFamilyIndex};
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipLevelCount;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = useFlags;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = 2;
        imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();

        VkImage image;
        vkCheck(vkCreateImage(device, &imageCreateInfo, nullptr, &image), {'R', 222});

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, image, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocInfo = {};
        memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocInfo.allocationSize = memoryRequirements.size;
        memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, propFlags);

        vkCheck(vkAllocateMemory(device, &memoryAllocInfo, nullptr, imageMemory), {'R', 222});
        vkBindImageMemory(device, image, *imageMemory, 0);

        return image;
    }

    uint32_t Renderer::rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &props);
        vkGetPhysicalDeviceFeatures(device, &features);

        int score = 0;

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;

        score += props.limits.maxImageDimension2D;

        if (!features.samplerAnisotropy)
            return 0;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int32_t graphicsFamily = -1;
        int32_t presentationFamily = -1;
        int i = 0;
        for (const VkQueueFamilyProperties &queueFamily : queueFamilies)
        {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsFamily = i;

            VkBool32 presentationSupport = false;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport) != VK_SUCCESS)
                Log::add('R', 214);

            if (queueFamily.queueCount > 0 && presentationSupport)
                presentationFamily = i;

            if (graphicsFamily >= 0 && presentationFamily >= 0)
                break;

            i++;
        }

        if (graphicsFamily < 0 || presentationFamily < 0)
            return 0;

        return score;
    }

    void Renderer::destroyImageAttachment(ImageAttachment &attachment) const
    {
        if (attachment.imageView)
            vkDestroyImageView(device, attachment.imageView, nullptr);
        if (attachment.image)
            vkDestroyImage(device, attachment.image, nullptr);
        if (attachment.memory)
            vkFreeMemory(device, attachment.memory, nullptr);

        attachment.imageView = VK_NULL_HANDLE;
        attachment.image = VK_NULL_HANDLE;
        attachment.memory = VK_NULL_HANDLE;
    }
}