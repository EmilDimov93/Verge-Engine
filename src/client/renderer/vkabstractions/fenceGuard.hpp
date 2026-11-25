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
            if (fence)
                vkDestroyFence(device, fence, nullptr);
            fence = VK_NULL_HANDLE;
        }

        [[nodiscard]] operator VkFence() const { return fence; }
        [[nodiscard]] const VkFence *ptr() const { return &fence; }

        FenceGuard(const FenceGuard &) = delete;
        FenceGuard &operator=(const FenceGuard &) = delete;

        FenceGuard(FenceGuard &&other) noexcept
            : fence(other.fence), device(other.device)
        {
            other.fence = VK_NULL_HANDLE;
        }
        FenceGuard &operator=(FenceGuard &&) = delete;

    private:
        VkFence fence = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
    };
}