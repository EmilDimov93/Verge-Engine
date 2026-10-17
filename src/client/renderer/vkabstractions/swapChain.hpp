// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace VE
{
    struct SwapChain
    {
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        VkExtent2D extent;
        uint32_t imageCount;
        VkFormat imageFormat = VK_FORMAT_UNDEFINED;
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

        operator VkSwapchainKHR() const { return swapChain; }
    };
}