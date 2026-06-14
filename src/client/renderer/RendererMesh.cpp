// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

namespace VE
{
    void Renderer::createVertexBuffer(MeshBuffer &meshBuffer, const std::vector<Vertex> &vertices)
    {
        VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        void *data;
        vkCheck(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data), {'V', 236});
        memcpy(data, vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &meshBuffer.vertexBuffer, &meshBuffer.vertexBufferMemory);

        CommandPoolGuard transferCommandPoolLocal(device, transferQueueFamilyIndex);

        VkFence uploadFence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &uploadFence), {'V', 216});

        vkCheck(copyBuffer(transferCommandPoolLocal, stagingBuffer, meshBuffer.vertexBuffer, bufferSize, uploadFence), {'V', 224});

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        vkDestroyFence(device, uploadFence, nullptr);
    }

    void Renderer::createIndexBuffer(MeshBuffer &meshBuffer, const std::vector<uint32_t> &indices)
    {
        VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

        void *data;
        vkCheck(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data), {'V', 236});
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &meshBuffer.indexBuffer, &meshBuffer.indexBufferMemory);

        CommandPoolGuard transferCommandPoolLocal(device, transferQueueFamilyIndex);

        VkFence uploadFence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &uploadFence), {'V', 216});

        vkCheck(copyBuffer(transferCommandPoolLocal, stagingBuffer, meshBuffer.indexBuffer, bufferSize, uploadFence), {'V', 224});

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        vkDestroyFence(device, uploadFence, nullptr);
    }

    void Renderer::destroyMeshBuffer(MeshBuffer &meshBuffer) const
    {
        if (meshBuffer.vertexBuffer)
            vkDestroyBuffer(device, meshBuffer.vertexBuffer, nullptr);
        if (meshBuffer.vertexBufferMemory)
            vkFreeMemory(device, meshBuffer.vertexBufferMemory, nullptr);

        if (meshBuffer.indexBuffer)
            vkDestroyBuffer(device, meshBuffer.indexBuffer, nullptr);
        if (meshBuffer.indexBufferMemory)
            vkFreeMemory(device, meshBuffer.indexBufferMemory, nullptr);

        meshBuffer.vertexBuffer = VK_NULL_HANDLE;
        meshBuffer.vertexBufferMemory = VK_NULL_HANDLE;
        meshBuffer.indexBuffer = VK_NULL_HANDLE;
        meshBuffer.indexBufferMemory = VK_NULL_HANDLE;
    }

}