// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "imageAttachment.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace VE
{
    struct TextureResources
    {
        std::vector<ImageAttachment> attachments;
        VkSampler sampler = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorPool> descriptorPools;
        std::vector<VkDescriptorSet> descriptorSets;
    };
}