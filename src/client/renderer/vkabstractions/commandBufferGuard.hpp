// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../vkCheck.hpp"

#include "../../../shared/log.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>

namespace VE
{
    class CommandBufferGuard
    {
    public:
        CommandBufferGuard(VkDevice device, uint32_t queueFamilyIndex) : device(device)
        {
            VkCommandPoolCreateInfo poolCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queueFamilyIndex};
            vkCheck(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &pool), {'R', 208});

            VkCommandBufferAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1};

            vkCheck(vkAllocateCommandBuffers(device, &allocInfo, &buffer), {'R', 212});
        }

        ~CommandBufferGuard()
        {
            if (buffer)
                vkFreeCommandBuffers(device, pool, 1, &buffer);
            buffer = VK_NULL_HANDLE;

            if (pool)
                vkDestroyCommandPool(device, pool, nullptr);
            pool = VK_NULL_HANDLE;
        }

        [[nodiscard]] operator VkCommandBuffer() const { return buffer; }
        [[nodiscard]] const VkCommandBuffer *ptr() const { return &buffer; }

        CommandBufferGuard(const CommandBufferGuard &) = delete;
        CommandBufferGuard &operator=(const CommandBufferGuard &) = delete;
        CommandBufferGuard(CommandBufferGuard &&other) noexcept
            : pool(other.pool), buffer(other.buffer), device(other.device)
        {
            other.pool = VK_NULL_HANDLE;
            other.buffer = VK_NULL_HANDLE;
        }
        CommandBufferGuard &operator=(CommandBufferGuard &&) = delete;

    private:
        VkCommandPool pool = VK_NULL_HANDLE;
        VkCommandBuffer buffer = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
    };
}