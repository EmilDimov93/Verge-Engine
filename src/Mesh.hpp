// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "definitions.hpp"

#define VE_SUCCESS VK_SUCCESS

class Mesh
{
public:
    VkResult init(VkPhysicalDevice newPhysicalDevice,
         VkDevice newDevice,
         VkQueue transferQueue,
         VkCommandPool transferCommandPool,
         std::vector<Vertex> *vertices,
         std::vector<uint32_t> *indeces);

    int getVertexCount();
    VkBuffer getVertexBuffer();

    int getIndexCount();
    VkBuffer getIndexBuffer();

    void destroyBuffers();

private:
    int vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    int indexCount;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> *vertices);
    void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t> *indeces);
};