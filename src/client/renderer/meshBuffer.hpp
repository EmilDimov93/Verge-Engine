// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
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
}