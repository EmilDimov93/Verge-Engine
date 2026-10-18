// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

namespace VE
{
    void Renderer::createSwapChain(glm::uvec2 windowSize)
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        swapChain.extent = {clamp(windowSize.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                            clamp(windowSize.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = capabilities.minImageCount + (capabilities.minImageCount != capabilities.maxImageCount),
            .imageFormat = swapChain.imageFormat,
            .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = swapChain.extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = swapChain.presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE};

        vkCheck(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain.swapChain), {'R', 204});

        vkGetSwapchainImagesKHR(device, swapChain, &swapChain.imageCount, nullptr);
        swapChain.images.resize(swapChain.imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &swapChain.imageCount, swapChain.images.data());

        swapChain.imageViews.resize(swapChain.imageCount);
        for (size_t i = 0; i < swapChain.imageCount; i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = swapChain.images[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = swapChain.imageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            vkCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapChain.imageViews[i]), {'R', 205});
        }
    }

    void Renderer::destroySwapChain(SwapChain swapChain) const
    {
        for (VkImageView view : swapChain.imageViews)
            if (view)
                vkDestroyImageView(device, view, nullptr);
        swapChain.imageViews.clear();

        if (swapChain)
            vkDestroySwapchainKHR(device, swapChain, nullptr);

        swapChain.swapChain = VK_NULL_HANDLE;
    }

    void Renderer::recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        for (GeometryBuffer &gBuffers : gBuffer.arr)
        {
            destroyImageAttachment(gBuffers.depth);
            destroyImageAttachment(gBuffers.baseColor);
            destroyImageAttachment(gBuffers.normal);
            destroyImageAttachment(gBuffers.metallic);
            destroyImageAttachment(gBuffers.roughness);
        }

        destroySwapChain(swapChain);

        for (ImageAttachment &attachment : prePostAttachments)
            destroyImageAttachment(attachment);

        createSwapChain(glm::uvec2(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
        createPrePostImages();
        updatePostDescriptorSets();
        createGeometryBuffers();
    }
}