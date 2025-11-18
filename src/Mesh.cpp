// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Mesh.hpp"

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

    std::cout << "No suitable memory type index";
    return -1;
}

void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory){
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = bufferUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    // should vkCheck()
    VkResult res = vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer);
    if (res != VK_SUCCESS)
    {
        std::cout << "Create buffer error";
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, bufferPropertyFlags)};

    res = vkAllocateMemory(device, &memoryAllocInfo, nullptr, bufferMemory);
    if (res != VK_SUCCESS)
    {
        std::cout << "Failed to allocate vertex buffer memory";
        exit(EXIT_FAILURE);
    }

    res = vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
    if (res != VK_SUCCESS)
    {
        std::cout << "Failed to bind buffer memory";
        exit(EXIT_FAILURE);
    }
}

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> *vertices)
{
    vertexCount = vertices->size();
    physicalDevice = newPhysicalDevice;
    device = newDevice;
    createVertexBuffer(transferQueue, transferCommandPool, vertices);
}

Mesh::Mesh() : vertexCount(0),
               vertexBuffer(VK_NULL_HANDLE),
               vertexBufferMemory(VK_NULL_HANDLE),
               physicalDevice(VK_NULL_HANDLE),
               device(VK_NULL_HANDLE)
{}

int Mesh::getVertexCount()
{
    return vertexCount;
}

VkBuffer Mesh::getVertexBuffer()
{
    return vertexBuffer;
}

void Mesh::destroyVertexBuffer()
{
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBufer, VkBuffer dstBuffer, VkDeviceSize bufferSize){
    VkCommandBuffer transferCommandBuffer;

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = transferCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    // should vkCheck
    VkResult res = vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer);
    if(res != VK_SUCCESS){
        std::cout << "Failed to allocate command buffers";
        exit(EXIT_FAILURE);
    }

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    res = vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);
    if(res != VK_SUCCESS){
        std::cout << "Failed to begin command buffer";
        exit(EXIT_FAILURE);
    }

    VkBufferCopy bufferCopyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = bufferSize
    };

    vkCmdCopyBuffer(transferCommandBuffer, srcBufer, dstBuffer, 1, &bufferCopyRegion);

    res = vkEndCommandBuffer(transferCommandBuffer);
    if(res != VK_SUCCESS){
        std::cout << "Failed to end command buffer";
        exit(EXIT_FAILURE);
    }

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &transferCommandBuffer
    };

    res = vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if(res != VK_SUCCESS){
        std::cout << "Failed to submit queue";
        exit(EXIT_FAILURE);
    }

    res = vkQueueWaitIdle(transferQueue);
    if(res != VK_SUCCESS){
        std::cout << "Failed to wait idle";
        exit(EXIT_FAILURE);
    }

    vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> *vertices)
{
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void *data;
    VkResult res = vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    if (res != VK_SUCCESS)
    {
        std::cout << "Failed to map memory";
        exit(EXIT_FAILURE);
    }
    memcpy(data, vertices->data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

    copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}