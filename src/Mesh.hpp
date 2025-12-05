// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "definitions.hpp"

struct UboModel
{
    glm::mat4 model;
};

class Mesh
{
public:
    VkResult init(VkPhysicalDevice newPhysicalDevice,
         VkDevice newDevice,
         VkQueue transferQueue,
         VkCommandPool transferCommandPool,
         std::vector<Vertex>* vertices,
         std::vector<uint32_t>* indeces);

    void setModel(glm::mat4 newModel);
    UboModel getModel();

    uint64_t getVertexCount();
    VkBuffer getVertexBuffer();

    uint64_t getIndexCount();
    VkBuffer getIndexBuffer();

    void destroyBuffers();

private:
    UboModel uboModel;

    uint64_t vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    uint64_t indexCount;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkResult createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
    VkResult createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indeces);
};