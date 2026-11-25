// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "vkabstractions/commandBufferGuard.hpp"
#include "vkabstractions/fenceGuard.hpp"

namespace VE
{
    struct UploadContext
    {
        CommandBufferGuard graphics;
        CommandBufferGuard transfer;
        FenceGuard fence;

        UploadContext(VkDevice device, uint32_t graphicsQueueFamilyIndex, uint32_t transferQueueFamilyIndex) : graphics(device, graphicsQueueFamilyIndex),
                                                                                                               transfer(device, transferQueueFamilyIndex),
                                                                                                               fence(device) {}
    };
}