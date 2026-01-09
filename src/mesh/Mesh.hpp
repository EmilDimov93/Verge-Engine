// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../definitions.hpp"

#include <vulkan/vulkan.h>
#include <vector>

struct Vertex
{
    glm::vec3 pos;
    ve_color_t col;

    Vertex(const glm::vec3 &position = glm::vec3(0.0f), const ve_color_t &color = ve_color_t(1.0f)) : pos(position), col(color) {}
};

struct VulkanContext
{
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
};

class Mesh
{
public:
    Mesh(VulkanContext vulkanContext, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

    uint64_t getVertexCount() const;
    VkBuffer getVertexBuffer() const;

    uint64_t getIndexCount() const;
    VkBuffer getIndexBuffer() const;

    void destroyBuffers(VkDevice device);

private:
    uint64_t vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    uint64_t indexCount;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    void createVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, const std::vector<Vertex> &vertices);
    void createIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, const std::vector<uint32_t> &indices);
};

struct MeshInstance
{
    uint32_t meshIndex;
    glm::mat4 model;
};

struct DrawData
{
    const std::vector<Mesh> &meshes;
    const std::vector<MeshInstance> &meshInstances;

    glm::mat4 projectionM;
    glm::mat4 viewM;

    ve_color_t backgroundColor;

    DrawData(const std::vector<Mesh> &newMeshes,
             const std::vector<MeshInstance> &newMeshInstances,
             glm::mat4 newProjectionM,
             glm::mat4 newViewM,
             ve_color_t newBackgroundColor)
        : meshes(newMeshes), meshInstances(newMeshInstances), projectionM(newProjectionM), viewM(newViewM), backgroundColor(newBackgroundColor) {}
};