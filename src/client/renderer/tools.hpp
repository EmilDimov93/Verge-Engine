// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/log.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <fstream>
#include <utility>

namespace VE
{
    template <typename... Structs>
    void *chainStructs(Structs *...structures)
    {
        void *arr[]{structures...};
        VkBaseInStructure *last = nullptr;
        for (auto *s : arr)
        {
            auto *vk_base = static_cast<VkBaseInStructure *>(s);
            vk_base->pNext = last;
            last = vk_base;
        }
        return last;
    }

    [[nodiscard]] inline uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            if ((allowedTypes & (1u << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            {
                return i;
            }
        }

        Log::add('R', 237);
        std::unreachable();
    }

    [[nodiscard]] inline std::vector<char> readFile(const std::string &fileName)
    {
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);

        if (!file.is_open())
            return {};

        size_t fileSize = (size_t)file.tellg();

        std::vector<char> fileBuffer(fileSize);

        file.seekg(0);

        file.read(fileBuffer.data(), fileSize);

        file.close();

        return fileBuffer;
    }
}