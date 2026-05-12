// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#define STB_IMAGE_IMPLEMENTATION

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

#include "../../../ext/stb_image/stb_image.h"

namespace VE
{
    stbi_uc *loadTextureFile(std::string fileName, int *width, int *height, VkDeviceSize *imageSize)
    {
        int channelCount;

        stbi_uc *image = stbi_load(fileName.c_str(), width, height, &channelCount, STBI_rgb_alpha);

        if (!image)
            Log::add('V', 225); // Should be warning?

        *imageSize = (*width) * (*height) * STBI_rgb_alpha;

        return image;
    }

    VkResult copyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height, std::mutex &transferQueueMutex, VkFence fence)
    {
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = transferCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};

        VkCommandBuffer transferCommandBuffer;

        VkResult res = vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer);
        if (res != VK_SUCCESS)
            return res;

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        res = vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);
        if (res != VK_SUCCESS)
            return res;

        VkBufferImageCopy imageRegion = {};
        imageRegion.bufferOffset = 0;
        imageRegion.bufferRowLength = 0;
        imageRegion.bufferImageHeight = 0;
        imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageRegion.imageSubresource.mipLevel = 0;
        imageRegion.imageSubresource.baseArrayLayer = 0;
        imageRegion.imageSubresource.layerCount = 1;
        imageRegion.imageOffset = {0, 0, 0};
        imageRegion.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

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

    VkResult transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, std::mutex &graphicsQueueMutex, VkFence fence)
    {
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};

        VkCommandBuffer commandBuffer;

        VkResult res = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
        if (res != VK_SUCCESS)
            return res;

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        if (res != VK_SUCCESS)
            return res;

        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = oldLayout;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        res = vkEndCommandBuffer(commandBuffer);
        if (res != VK_SUCCESS)
            return res;

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer};

        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            res = vkQueueSubmit(queue, 1, &submitInfo, fence);
            if (res != VK_SUCCESS)
                return res;
        }

        res = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
        if (res != VK_SUCCESS)
            return res;

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

        return VK_SUCCESS;
    }

    void Renderer::createFallbackTexture()
    {
        uint32_t whitePixel = 0xFFFFFFFF;
        VkDeviceSize imageSize = 4;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        void *data;
        vkCheck(vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data), {'V', 236});
        memcpy(data, &whitePixel, imageSize);
        vkUnmapMemory(device, stagingBufferMemory);

        VkImage texImage;
        VkDeviceMemory texImageMemory;
        texImage = createImage(1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

        VkCommandPool graphicsCommandPoolLocal = VK_NULL_HANDLE;
        VkCommandPoolCreateInfo graphicsPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = graphicsQueueFamilyIndex};
        vkCheck(vkCreateCommandPool(device, &graphicsPoolCreateInfo, nullptr, &graphicsCommandPoolLocal), {'V', 208});

        VkCommandPool transferCommandPoolLocal = VK_NULL_HANDLE;
        VkCommandPoolCreateInfo transferPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = transferQueueFamilyIndex};
        vkCheck(vkCreateCommandPool(device, &transferPoolCreateInfo, nullptr, &transferCommandPoolLocal), {'V', 208});

        VkFence uploadFence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &uploadFence), {'V', 216});

        vkCheck(transitionImageLayout(device, graphicsQueue, graphicsCommandPoolLocal, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, graphicsQueueMutex, uploadFence), {'V', 240});
        vkCheck(vkResetFences(device, 1, &uploadFence), {'V', 232});
        vkCheck(copyImageBuffer(device, transferQueue, transferCommandPoolLocal, stagingBuffer, texImage, 1, 1, transferQueueMutex, uploadFence), {'V', 239});
        vkCheck(vkResetFences(device, 1, &uploadFence), {'V', 232});
        vkCheck(transitionImageLayout(device, graphicsQueue, graphicsCommandPoolLocal, texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, graphicsQueueMutex, uploadFence), {'V', 240});

        {
            std::lock_guard<std::mutex> lock(textureMutex);
            textureImages.push_back(texImage);
            textureImageMemory.push_back(texImageMemory);
        }

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        vkDestroyFence(device, uploadFence, nullptr);
        vkDestroyCommandPool(device, graphicsCommandPoolLocal, nullptr);
        vkDestroyCommandPool(device, transferCommandPoolLocal, nullptr);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = texImage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        imageViewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkImageView imageView;
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView), {'V', 205});

        {
            std::lock_guard<std::mutex> lock(textureMutex);
            textureImageViews.push_back(imageView);
            createTextureDescriptor(imageView);
        }
    }

    size_t Renderer::createTextureImage(std::string fileName)
    {
        int width, height;
        VkDeviceSize imageSize;

        stbi_uc *imageData = loadTextureFile(fileName, &width, &height, &imageSize);

        VkBuffer imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &imageStagingBuffer, &imageStagingBufferMemory);

        void *data;
        vkCheck(vkMapMemory(device, imageStagingBufferMemory, 0, imageSize, 0, &data), {'V', 236});
        memcpy(data, imageData, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, imageStagingBufferMemory);

        stbi_image_free(imageData);

        VkImage texImage;
        VkDeviceMemory texImageMemory;

        texImage = createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);

        VkCommandPool graphicsCommandPoolLocal = VK_NULL_HANDLE;
        VkCommandPoolCreateInfo graphicsPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = graphicsQueueFamilyIndex};
        vkCheck(vkCreateCommandPool(device, &graphicsPoolCreateInfo, nullptr, &graphicsCommandPoolLocal), {'V', 208});

        VkCommandPool transferCommandPoolLocal = VK_NULL_HANDLE;
        VkCommandPoolCreateInfo transferPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = transferQueueFamilyIndex};
        vkCheck(vkCreateCommandPool(device, &transferPoolCreateInfo, nullptr, &transferCommandPoolLocal), {'V', 208});

        VkFence uploadFence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &uploadFence), {'V', 216});

        vkCheck(transitionImageLayout(device, graphicsQueue, graphicsCommandPoolLocal, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, graphicsQueueMutex, uploadFence), {'V', 240});
        vkCheck(vkResetFences(device, 1, &uploadFence), {'V', 232});
        vkCheck(copyImageBuffer(device, transferQueue, transferCommandPoolLocal, imageStagingBuffer, texImage, width, height, transferQueueMutex, uploadFence), {'V', 239});
        vkCheck(vkResetFences(device, 1, &uploadFence), {'V', 232});
        vkCheck(transitionImageLayout(device, graphicsQueue, graphicsCommandPoolLocal, texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, graphicsQueueMutex, uploadFence), {'V', 240});

        size_t resultIndex;
        {
            std::lock_guard<std::mutex> lock(textureMutex);
            textureImages.push_back(texImage);
            textureImageMemory.push_back(texImageMemory);
            resultIndex = textureImages.size() - 1;
        }

        vkDestroyBuffer(device, imageStagingBuffer, nullptr);
        vkFreeMemory(device, imageStagingBufferMemory, nullptr);

        vkDestroyFence(device, uploadFence, nullptr);
        vkDestroyCommandPool(device, graphicsCommandPoolLocal, nullptr);
        vkDestroyCommandPool(device, transferCommandPoolLocal, nullptr);

        return resultIndex;
    }

    size_t Renderer::createTexture(std::string fileName)
    {
        // Temporary
        {
            std::lock_guard<std::mutex> lock(textureMutex);
            if (samplerDescriptorSets.size() >= MAX_OBJECTS)
            {
                Log::add('V', 120);
                return INVALID_TEXTURE_INDEX;
            }
        }

        size_t textureImageLoc = createTextureImage(fileName);

        VkImage sourceImage;
        {
            std::lock_guard<std::mutex> lock(textureMutex);
            sourceImage = textureImages[textureImageLoc];
        }

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = sourceImage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView), {'V', 205});

        {
            std::lock_guard<std::mutex> lock(textureMutex);
            textureImageViews.push_back(imageView);
            return createTextureDescriptor(imageView);
        }
    }

    size_t Renderer::createTextureDescriptor(VkImageView textureImageView)
    {
        VkDescriptorSet descriptorSet;

        VkDescriptorSetAllocateInfo setAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = samplerDescriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &samplerSetLayout};

        vkCheck(vkAllocateDescriptorSets(device, &setAllocInfo, &descriptorSet), {'V', 220});

        VkDescriptorImageInfo imageInfo = {
            .sampler = textureSampler,
            .imageView = textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo};

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

        samplerDescriptorSets.push_back(descriptorSet);

        return samplerDescriptorSets.size() - 1;
    }

    void Renderer::createTextureSampler()
    {
        VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE};

        vkCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &textureSampler), {'V', 226});
    }
}