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
         std::vector<Vertex> *vertices);

    int getVertexCount();

    VkBuffer getVertexBuffer();

    void destroyVertexBuffer();

private:
    int vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> *vertices);
};