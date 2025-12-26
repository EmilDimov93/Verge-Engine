// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../definitions.hpp"

#include <vulkan/vulkan.h>
#include <vector>

struct VulkanContext{
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
};

struct Model
{
    glm::mat4 model;
};

class Mesh
{
public:
    VkResult init(VulkanContext vulkanContext, std::vector<Vertex>* vertices, std::vector<uint32_t>* indeces);

    void setModel(glm::mat4 newModel);
    Model getModel() const;

    uint64_t getVertexCount() const;
    VkBuffer getVertexBuffer() const;

    uint64_t getIndexCount() const;
    VkBuffer getIndexBuffer() const;

    void destroyBuffers(VkDevice device);

private:
    Model model;

    uint64_t vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    uint64_t indexCount;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkResult createVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
    VkResult createIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indeces);
};