// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

#include <fstream>
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
            Log::add('V', 100);
            break;
        case VK_TIMEOUT:
            Log::add('V', 101);
            break;
        case VK_SUBOPTIMAL_KHR:
            Log::add('V', 102);
            break;
        case VK_EVENT_SET:
            Log::add('V', 103);
            break;
        case VK_EVENT_RESET:
            Log::add('V', 104);
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

        Log::add('V', 237);
        return -1;
    }

    std::vector<char> Renderer::readFile(const std::string &fileName)
    {
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);

        if (!file.is_open())
        {
            return {};
        }

        size_t fileSize = (size_t)file.tellg();

        std::vector<char> fileBuffer(fileSize);

        file.seekg(0);

        file.read(fileBuffer.data(), fileSize);

        file.close();

        return fileBuffer;
    }

    VkShaderModule Renderer::createShaderModule(const std::vector<char> &code) const
    {
        if (code.empty())
            Log::add('V', 221);

        VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t *>(code.data())};

        VkShaderModule shaderModule;

        vkCheck(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule), {'V', 209});

        return shaderModule;
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

        vkCheck(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer), {'V', 218});

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

        VkMemoryAllocateInfo memoryAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, bufferPropertyFlags)};

        vkCheck(vkAllocateMemory(device, &memoryAllocInfo, nullptr, bufferMemory), {'V', 218});

        vkCheck(vkBindBufferMemory(device, *buffer, *bufferMemory, 0), {'V', 218});
    }

    VkResult Renderer::copyBuffer(VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkFence fence) const
    {
        VkResult res;

        VkCommandBuffer transferCommandBuffer;

        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = transferCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};

        res = vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer);
        if (res != VK_SUCCESS)
            return res;

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        res = vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);
        if (res != VK_SUCCESS)
            return res;

        VkBufferCopy bufferCopyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = bufferSize};

        vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

        res = vkEndCommandBuffer(transferCommandBuffer);
        if (res != VK_SUCCESS)
            return res;

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &transferCommandBuffer};

        {
            std::lock_guard<std::mutex> lock(transferQueueMutex);
            res = vkQueueSubmit(transferQueue, 1, &submitInfo, fence);
            if (res != VK_SUCCESS)
                return res;
        }

        res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
        if (res != VK_SUCCESS)
            return res;

        vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);

        return VK_SUCCESS;
    }

    VkImage Renderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory *imageMemory)
    {
        std::array<uint32_t, 2> queueFamilyIndices = {graphicsQueueFamilyIndex, transferQueueFamilyIndex};
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
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
        vkCheck(vkCreateImage(device, &imageCreateInfo, nullptr, &image), {'V', 222});

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, image, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocInfo = {};
        memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocInfo.allocationSize = memoryRequirements.size;
        memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, propFlags);

        vkCheck(vkAllocateMemory(device, &memoryAllocInfo, nullptr, imageMemory), {'V', 222});
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
                Log::add('V', 214);

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
    }
}