// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Mesh.hpp"

Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, std::vector<Vertex> *vertices)
{
    vertexCount = vertices->size();
    physicalDevice = newPhysicalDevice;
    device = newDevice;
    createVertexBuffer(vertices);
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

void Mesh::createVertexBuffer(std::vector<Vertex> *vertices)
{
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(Vertex) * vertices->size(),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    // should vkCheck()
    VkResult res = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &vertexBuffer);
    if (res != VK_SUCCESS)
    {
        std::cout << "Create buffer error";
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};

    res = vkAllocateMemory(device, &memoryAllocInfo, nullptr, &vertexBufferMemory);
    if (res != VK_SUCCESS)
    {
        std::cout << "Failed to allocate vertex buffer memory";
        exit(EXIT_FAILURE);
    }

    res = vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
    if (res != VK_SUCCESS)
    {
        std::cout << "Failed to bind buffer memory";
        exit(EXIT_FAILURE);
    }

    void *data;
    res = vkMapMemory(device, vertexBufferMemory, 0, bufferCreateInfo.size, 0, &data);
    if (res != VK_SUCCESS)
    {
        std::cout << "Failed to map memory";
        exit(EXIT_FAILURE);
    }
    memcpy(data, vertices->data(), (size_t)bufferCreateInfo.size);
    vkUnmapMemory(device, vertexBufferMemory);
}

void Mesh::destroyVertexBuffer()
{
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

uint32_t Mesh::findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties)
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