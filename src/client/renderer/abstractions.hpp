// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/drawData.hpp"

#include "../../shared/log.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <array>

namespace VE
{
    struct GraphicsPipeline
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets;
    };

    struct ImageAttachment
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        uint32_t mipLevelCount = 1;
    };

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

    struct ShadowMap
    {
        ModelInstanceHandle instanceHandle;

        ImageAttachment depthAttachment;
    };

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

    struct TextureResources
    {
        std::vector<ImageAttachment> attachments;
        VkSampler sampler = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorPool> descriptorPools;
        std::vector<VkDescriptorSet> descriptorSets;
    };

    struct GeometryBuffer
    {
        ImageAttachment depth;
        ImageAttachment normal;
        ImageAttachment metallic;
        ImageAttachment roughness;
    };

    struct MeshBuffer
    {
        uint32_t vertexCount = 0;
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

        uint32_t indexCount = 0;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

        uint32_t materialIndex = 0;

        // Used for Models, ignored for Widgets
        bool isTransparent = false;
    };

    struct ModelBuffer
    {
        ModelHandle handle;

        std::vector<MeshBuffer> meshBuffers;
        std::vector<Material> materials;

        uint64_t version = 0;

        ModelBuffer(ModelHandle handle) : handle(handle) {}
    };

    struct WidgetBuffer
    {
        WidgetHandle handle;

        std::vector<MeshBuffer> meshBuffers;
        std::vector<Material> materials;

        uint64_t version = 0;

        WidgetBuffer(WidgetHandle handle) : handle(handle) {}
    };
}