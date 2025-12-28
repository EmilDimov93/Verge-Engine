// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Mesh.hpp"

#include "../Log.hpp"

#define VK_CHECK(res)           \
    do                          \
    {                           \
        if (res != VK_SUCCESS)  \
            Log::add('V', 229); \
    } while (0)

void Mesh::init(VulkanContext vulkanContext, std::vector<Vertex> *vertices, std::vector<uint32_t> *indeces)
{
    vertexCount = vertices->size();
    indexCount = indeces->size();
    createVertexBuffer(vulkanContext.physicalDevice, vulkanContext.device, vulkanContext.graphicsQueue, vulkanContext.graphicsCommandPool, vertices);
    createIndexBuffer(vulkanContext.physicalDevice, vulkanContext.device, vulkanContext.graphicsQueue, vulkanContext.graphicsCommandPool, indeces);

    model = glm::mat4(1.0f);
}

void Mesh::setModel(glm::mat4 newModel)
{
    model = newModel;
}

glm::mat4 Mesh::getModel() const
{
    return model;
}

uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((allowedTypes & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
        {
            return i;
        }
    }

    Log::add('V', 237);
    return -1;
}

void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory)
{
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = bufferUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    VK_CHECK(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, bufferPropertyFlags)};

    VK_CHECK(vkAllocateMemory(device, &memoryAllocInfo, nullptr, bufferMemory));

    VK_CHECK(vkBindBufferMemory(device, *buffer, *bufferMemory, 0));
}

uint64_t Mesh::getVertexCount() const
{
    return vertexCount;
}

VkBuffer Mesh::getVertexBuffer() const
{
    return vertexBuffer;
}

uint64_t Mesh::getIndexCount() const
{
    return indexCount;
}

VkBuffer Mesh::getIndexBuffer() const
{
    return indexBuffer;
}

void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBufer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
    VkCommandBuffer transferCommandBuffer;

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = transferCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer));

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VK_CHECK(vkBeginCommandBuffer(transferCommandBuffer, &beginInfo));

    VkBufferCopy bufferCopyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = bufferSize};

    vkCmdCopyBuffer(transferCommandBuffer, srcBufer, dstBuffer, 1, &bufferCopyRegion);

    VK_CHECK(vkEndCommandBuffer(transferCommandBuffer));

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &transferCommandBuffer};

    VK_CHECK(vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE));

    VK_CHECK(vkQueueWaitIdle(transferQueue));

    vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);
}

void Mesh::createVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> *vertices)
{
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void *data;
    VK_CHECK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data));
    memcpy(data, vertices->data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

    copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Mesh::createIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t> *indeces)
{
    VkDeviceSize bufferSize = sizeof(uint32_t) * indeces->size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void *data;
    VK_CHECK(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data));
    memcpy(data, indeces->data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

    copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Mesh::destroyBuffers(VkDevice device)
{
    if (vertexBuffer)
        vkDestroyBuffer(device, vertexBuffer, nullptr);
    if (vertexBufferMemory)
        vkFreeMemory(device, vertexBufferMemory, nullptr);

    if (indexBuffer)
        vkDestroyBuffer(device, indexBuffer, nullptr);
    if (indexBufferMemory)
        vkFreeMemory(device, indexBufferMemory, nullptr);

    vertexBuffer = VK_NULL_HANDLE;
    vertexBufferMemory = VK_NULL_HANDLE;
    indexBuffer = VK_NULL_HANDLE;
    indexBufferMemory = VK_NULL_HANDLE;
}