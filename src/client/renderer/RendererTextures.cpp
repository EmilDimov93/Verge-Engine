// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

namespace VE
{
    VkResult copyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height, std::mutex &transferQueueMutex, VkFence fence)
    {
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = transferCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};

        VkCommandBuffer transferCommandBuffer;

        Renderer::vkCheck(vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer), {'R', 212});

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        Renderer::vkCheck(vkBeginCommandBuffer(transferCommandBuffer, &beginInfo), {'R', 213});

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

        Renderer::vkCheck(vkEndCommandBuffer(transferCommandBuffer), {'R', 213});

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &transferCommandBuffer};

        {
            std::lock_guard<std::mutex> lock(transferQueueMutex);
            Renderer::vkCheck(vkQueueSubmit(transferQueue, 1, &submitInfo, fence), {'R', 233});
        }

        Renderer::vkCheck(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), {'R', 231});

        vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);

        return VK_SUCCESS;
    }

    void Renderer::createTextureDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &samplerLayoutBinding};

        vkCheck(vkCreateDescriptorSetLayout(device, &textureLayoutCreateInfo, nullptr, &textures.descriptorSetLayout), {'R', 217});
    }

    VkResult generateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue queue, VkCommandPool commandPool, VkImage image, VkFormat imageFormat, int32_t textureWidth, int32_t textureHeight, uint32_t mipLevelCount, std::mutex &graphicsQueueMutex, VkFence fence)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
            return VK_ERROR_FEATURE_NOT_PRESENT;

        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};

        VkCommandBuffer commandBuffer;
        Renderer::vkCheck(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer), {'R', 212});

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

        Renderer::vkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo), {'R', 213});

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        if (mipLevelCount > 1)
        {
            VkImageMemoryBarrier initBarrier = barrier;
            initBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            initBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            initBarrier.srcAccessMask = 0;
            initBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            initBarrier.subresourceRange.baseMipLevel = 1;
            initBarrier.subresourceRange.levelCount = mipLevelCount - 1;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &initBarrier);
        }

        int32_t currentMipWidth = textureWidth;
        int32_t currentMipHeight = textureHeight;

        for (uint32_t mipIndex = 1; mipIndex < mipLevelCount; mipIndex++)
        {
            barrier.subresourceRange.baseMipLevel = mipIndex - 1;
            barrier.subresourceRange.levelCount = 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            int32_t nextMipWidth = currentMipWidth > 1 ? currentMipWidth / 2 : 1;
            int32_t nextMipHeight = currentMipHeight > 1 ? currentMipHeight / 2 : 1;

            VkImageBlit blit = {};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {currentMipWidth, currentMipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = mipIndex - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {nextMipWidth, nextMipHeight, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = mipIndex;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            currentMipWidth = nextMipWidth;
            currentMipHeight = nextMipHeight;
        }

        barrier.subresourceRange.baseMipLevel = mipLevelCount - 1;
        barrier.subresourceRange.levelCount = 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        Renderer::vkCheck(vkEndCommandBuffer(commandBuffer), {'R', 213});

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer};

        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            Renderer::vkCheck(vkQueueSubmit(queue, 1, &submitInfo, fence), {'R', 233});
        }

        Renderer::vkCheck(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), {'R', 231});

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
        vkCheck(vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data), {'R', 236});
        memcpy(data, &whitePixel, imageSize);
        vkUnmapMemory(device, stagingBufferMemory);

        VkImage texImage;
        VkDeviceMemory texImageMemory;
        texImage = createImage(1, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, &texImageMemory);

        CommandPoolGuard graphicsCommandPoolLocal(device, graphicsQueueFamilyIndex);
        CommandPoolGuard transferCommandPoolLocal(device, transferQueueFamilyIndex);

        FenceGuard uploadFence(device);

        transitionImageLayout(device, graphicsQueue, graphicsCommandPoolLocal, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, graphicsQueueMutex, uploadFence);
        vkCheck(vkResetFences(device, 1, uploadFence.ptr()), {'R', 232});
        vkCheck(copyImageBuffer(device, transferQueue, transferCommandPoolLocal, stagingBuffer, texImage, 1, 1, transferQueueMutex, uploadFence), {'R', 239});
        vkCheck(vkResetFences(device, 1, uploadFence.ptr()), {'R', 232});
        transitionImageLayout(device, graphicsQueue, graphicsCommandPoolLocal, texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, graphicsQueueMutex, uploadFence);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = texImage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        imageViewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        VkImageView imageView;
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView), {'R', 205});

        {
            std::lock_guard<std::mutex> lock(textureMutex);
            textures.attachments.push_back({texImage, texImageMemory, imageView});
            (void)createTextureDescriptor(imageView);
        }

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    uint32_t Renderer::createTextureImage(const uint8_t *imageData, glm::uvec2 size)
    {
        if (!imageData || size.x <= 0 || size.y <= 0)
        {
            Log::add('R', 111);
            return INVALID_TEXTURE_INDEX;
        }

        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(size.x * size.y * 4);

        uint32_t mipLevelCount = static_cast<uint32_t>(std::floor(std::log2(std::max(size.x, size.y)))) + 1;

        VkBuffer imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &imageStagingBuffer, &imageStagingBufferMemory);

        void *data;
        vkCheck(vkMapMemory(device, imageStagingBufferMemory, 0, imageSize, 0, &data), {'R', 236});
        memcpy(data, imageData, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, imageStagingBufferMemory);

        VkImage texImage;
        VkDeviceMemory texImageMemory;

        texImage = createImage(size.x, size.y, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mipLevelCount, &texImageMemory);

        CommandPoolGuard graphicsCommandPoolLocal(device, graphicsQueueFamilyIndex);
        CommandPoolGuard transferCommandPoolLocal(device, transferQueueFamilyIndex);

        FenceGuard uploadFence(device);

        transitionImageLayout(device, graphicsQueue, graphicsCommandPoolLocal, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, graphicsQueueMutex, uploadFence);
        vkCheck(vkResetFences(device, 1, uploadFence.ptr()), {'R', 232});
        vkCheck(copyImageBuffer(device, transferQueue, transferCommandPoolLocal, imageStagingBuffer, texImage, size.x, size.y, transferQueueMutex, uploadFence), {'R', 239});
        vkCheck(vkResetFences(device, 1, uploadFence.ptr()), {'R', 232});
        vkCheck(generateMipmaps(device, physicalDevice, graphicsQueue, graphicsCommandPoolLocal, texImage, VK_FORMAT_R8G8B8A8_SRGB, size.x, size.y, mipLevelCount, graphicsQueueMutex, uploadFence), {'R', 240});

        uint32_t resultIndex;
        {
            std::lock_guard<std::mutex> lock(textureMutex);
            textures.attachments.push_back({texImage, texImageMemory, VK_NULL_HANDLE, mipLevelCount});
            resultIndex = static_cast<uint32_t>(textures.attachments.size() - 1);
        }

        vkDestroyBuffer(device, imageStagingBuffer, nullptr);
        vkFreeMemory(device, imageStagingBufferMemory, nullptr);

        return resultIndex;
    }

    uint32_t Renderer::createTexture(std::vector<uint8_t> texturePixels, glm::uvec2 textureSize)
    {
        uint32_t textureImageIndex = createTextureImage(texturePixels.data(), textureSize);

        if (textureImageIndex == INVALID_TEXTURE_INDEX)
            return INVALID_TEXTURE_INDEX;

        VkImage sourceImage;
        uint32_t mipLevelCount;
        {
            std::lock_guard<std::mutex> lock(textureMutex);
            sourceImage = textures.attachments[textureImageIndex].image;
            mipLevelCount = textures.attachments[textureImageIndex].mipLevelCount;
        }

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = sourceImage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = mipLevelCount;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView), {'R', 205});

        {
            std::lock_guard<std::mutex> lock(textureMutex);
            textures.attachments[textureImageIndex].imageView = imageView;
            return createTextureDescriptor(imageView);
        }
    }

    uint32_t Renderer::createTextureDescriptor(VkImageView textureImageView)
    {
        const auto createDescriptorPool = [&]()
        {
            VkDescriptorPoolSize poolSize = {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = TEXTURE_SAMPLER_POOL_CHUNK_SIZE};

            VkDescriptorPoolCreateInfo poolCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .maxSets = TEXTURE_SAMPLER_POOL_CHUNK_SIZE,
                .poolSizeCount = 1,
                .pPoolSizes = &poolSize};

            VkDescriptorPool pool;
            vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &pool);
            return pool;
        };

        if (textures.descriptorPools.empty())
            textures.descriptorPools.emplace_back(createDescriptorPool());

        VkDescriptorSet descriptorSet;

        VkDescriptorSetAllocateInfo setAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = textures.descriptorPools.back(),
            .descriptorSetCount = 1,
            .pSetLayouts = &textures.descriptorSetLayout};

        VkResult result = vkAllocateDescriptorSets(device, &setAllocInfo, &descriptorSet);

        if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
        {
            textures.descriptorPools.emplace_back(createDescriptorPool());

            setAllocInfo.descriptorPool = textures.descriptorPools.back();
            vkCheck(vkAllocateDescriptorSets(device, &setAllocInfo, &descriptorSet), {'R', 220});
        }
        else
        {
            vkCheck(result, {'R', 220});
        }

        VkDescriptorImageInfo imageInfo = {
            .sampler = textures.sampler,
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

        textures.descriptorSets.push_back(descriptorSet);

        return static_cast<uint32_t>(textures.descriptorSets.size() - 1);
    }

    void Renderer::createTextureSampler()
    {
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        const float maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

        VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = maxAnisotropy,
            .minLod = 0.f,
            .maxLod = VK_LOD_CLAMP_NONE,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE};

        vkCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &textures.sampler), {'R', 226});
    }
}