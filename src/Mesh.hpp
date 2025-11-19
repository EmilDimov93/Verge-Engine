// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include <vulkan/vulkan.h>
#include <vector>

#include "definitions.hpp"

class Mesh
{
public:
    Mesh();
    Mesh(VkPhysicalDevice newPhysicalDevice,
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