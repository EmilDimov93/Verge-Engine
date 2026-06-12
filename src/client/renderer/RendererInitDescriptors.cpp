// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

namespace VE
{
    void Renderer::createModelUniformBuffers()
    {
        VkDeviceSize cameraBufferSize = sizeof(UboCamera);
        VkDeviceSize lightingBufferSize = sizeof(UboLighting);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            createBuffer(cameraBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &cameraUniformBuffer[i], &cameraUniformBufferMemory[i]);
            createBuffer(lightingBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &lightingUniformBuffer[i], &lightingUniformBufferMemory[i]);
        }
    }

    void Renderer::createShadowSampler()
    {
        VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .anisotropyEnable = VK_FALSE,
            .compareEnable = VK_TRUE,
            .compareOp = VK_COMPARE_OP_LESS,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE};
        vkCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &shadowSampler), {'V', 218});
    }
    
    void Renderer::createModelDescriptors()
    {
        // Layout
        VkDescriptorSetLayoutBinding cameraLayoutBinding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutBinding lightingLayoutBinding = {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutBinding shadowLayoutBinding = {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr};

        std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings = {cameraLayoutBinding, lightingLayoutBinding, shadowLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data()};

        vkCheck(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &modelPipeline.descriptorSetLayout), {'V', 217});

        // Pool
            VkDescriptorPoolSize uniformPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(cameraUniformBuffer.size() + lightingUniformBuffer.size())};

        VkDescriptorPoolSize shadowPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = FRAMES_IN_FLIGHT};

        std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes = {uniformPoolSize, shadowPoolSize};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = FRAMES_IN_FLIGHT,
            .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
            .pPoolSizes = descriptorPoolSizes.data()};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &modelPipeline.descriptorPool), {'V', 219});

        // Sets
        modelPipeline.descriptorSets.resize(FRAMES_IN_FLIGHT);

        std::vector<VkDescriptorSetLayout> setLayouts(FRAMES_IN_FLIGHT, modelPipeline.descriptorSetLayout);

        VkDescriptorSetAllocateInfo setAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = modelPipeline.descriptorPool,
            .descriptorSetCount = FRAMES_IN_FLIGHT,
            .pSetLayouts = setLayouts.data()};

        vkCheck(vkAllocateDescriptorSets(device, &setAllocInfo, modelPipeline.descriptorSets.data()), {'V', 220});

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo cameraBufferInfo = {
                .buffer = cameraUniformBuffer[i],
                .offset = 0,
                .range = sizeof(UboCamera)};

            VkWriteDescriptorSet cameraSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = modelPipeline.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &cameraBufferInfo};

            VkDescriptorBufferInfo lightingBufferInfo = {
                .buffer = lightingUniformBuffer[i],
                .offset = 0,
                .range = sizeof(UboLighting)};

            VkWriteDescriptorSet lightingSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = modelPipeline.descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &lightingBufferInfo};

            VkDescriptorImageInfo shadowImageInfo = {
                .sampler = shadowSampler,
                .imageView = shadowDepthAttachment.imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

            VkWriteDescriptorSet shadowSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = modelPipeline.descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &shadowImageInfo};

            std::vector<VkWriteDescriptorSet> setWrites = {cameraSetWrite, lightingSetWrite, shadowSetWrite};

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
        }
    }

    void Renderer::createUIUniformBuffers()
    {
        VkDeviceSize uiUniformBufferSize = sizeof(UboUI);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            createBuffer(uiUniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uiUniformBuffers[i], &uiUniformBuffersMemory[i]);
    }

    void Renderer::createUIDescriptors()
    {
        // Layout
        VkDescriptorSetLayoutBinding uiLayoutBinding = {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutCreateInfo uiLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &uiLayoutBinding};

        vkCheck(vkCreateDescriptorSetLayout(device, &uiLayoutCreateInfo, nullptr, &uiPipeline.descriptorSetLayout), {'V', 217});

        // Pool
        VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(uiUniformBuffers.size())};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = FRAMES_IN_FLIGHT,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &uiPipeline.descriptorPool), {'V', 219});

        // Sets
        uiPipeline.descriptorSets.resize(FRAMES_IN_FLIGHT);

        std::vector<VkDescriptorSetLayout> setLayouts(FRAMES_IN_FLIGHT, uiPipeline.descriptorSetLayout);

        VkDescriptorSetAllocateInfo setAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = uiPipeline.descriptorPool,
            .descriptorSetCount = FRAMES_IN_FLIGHT,
            .pSetLayouts = setLayouts.data()};

        vkCheck(vkAllocateDescriptorSets(device, &setAllocInfo, uiPipeline.descriptorSets.data()), {'V', 220});

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo uiBufferInfo = {
                .buffer = uiUniformBuffers[i],
                .offset = 0,
                .range = sizeof(UboUI)};

            VkWriteDescriptorSet uiSetWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = uiPipeline.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &uiBufferInfo};

            vkUpdateDescriptorSets(device, 1, &uiSetWrite, 0, nullptr);
        }
    }

    void Renderer::createPostSampler()
    {
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.anisotropyEnable = VK_FALSE;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 0.0f;

        vkCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &postSampler), {'V', 226});
    }

    void Renderer::createPostDescriptors()
    {
        // Layout
        VkDescriptorSetLayoutBinding sceneColorBinding{};
        sceneColorBinding.binding = 0;
        sceneColorBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sceneColorBinding.descriptorCount = 1;
        sceneColorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        sceneColorBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &sceneColorBinding;

        vkCheck(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &postPipeline.descriptorSetLayout), {'V', 217});

        // Pool
        VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = static_cast<uint32_t>(swapChainImageCount)};

        VkDescriptorPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(swapChainImageCount),
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize};

        vkCheck(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &postPipeline.descriptorPool), {'V', 219});

        // Sets
        const size_t imageCount = swapChainImageCount;

        std::vector<VkDescriptorSetLayout> layouts(imageCount, postPipeline.descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = postPipeline.descriptorPool;
        allocateInfo.descriptorSetCount = static_cast<uint32_t>(imageCount);
        allocateInfo.pSetLayouts = layouts.data();

        postPipeline.descriptorSets.resize(imageCount);
        vkCheck(vkAllocateDescriptorSets(device, &allocateInfo, postPipeline.descriptorSets.data()), {'V', 220});

        updatePostDescriptorSets();
    }

    void Renderer::updatePostDescriptorSets()
    {
        for (size_t i = 0; i < postPipeline.descriptorSets.size(); i++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = prePostAttachments[i].imageView;
            imageInfo.sampler = postSampler;

            VkWriteDescriptorSet postSetWrite{};
            postSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            postSetWrite.dstSet = postPipeline.descriptorSets[i];
            postSetWrite.dstBinding = 0;
            postSetWrite.dstArrayElement = 0;
            postSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            postSetWrite.descriptorCount = 1;
            postSetWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, 1, &postSetWrite, 0, nullptr);
        }
    }
}