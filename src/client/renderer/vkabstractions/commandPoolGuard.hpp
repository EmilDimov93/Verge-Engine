// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../../shared/log.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>

namespace VE
{
    class CommandPoolGuard
    {
    public:
        CommandPoolGuard(VkDevice device, uint32_t queueFamilyIndex) : device(device)
        {
            VkCommandPoolCreateInfo poolCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = queueFamilyIndex};
            if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
                Log::add('R', 208);
        }

        ~CommandPoolGuard()
        {
            vkDestroyCommandPool(device, pool, nullptr);
        }

        [[nodiscard]] operator VkCommandPool() const { return pool; }

    private:
        VkCommandPool pool = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
    };
}