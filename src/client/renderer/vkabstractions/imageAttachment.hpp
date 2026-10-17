// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

namespace VE
{
    struct ImageAttachment
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        uint32_t mipLevelCount = 1;
    };
}