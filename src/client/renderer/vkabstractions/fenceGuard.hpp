// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../../shared/log.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>

namespace VE
{
    class FenceGuard
    {
    public:
        FenceGuard(VkDevice device) : device(device)
        {
            VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            if (vkCreateFence(device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
                Log::add('R', 216);
        }

        ~FenceGuard()
        {
            vkDestroyFence(device, fence, nullptr);
        }

        [[nodiscard]] operator VkFence() const { return fence; }

        [[nodiscard]] const VkFence *ptr() const { return &fence; }

    private:
        VkFence fence = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
    };
}