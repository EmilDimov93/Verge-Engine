// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

namespace VE
{
    void Renderer::syncModelBuffers(const std::vector<Model> &models)
    {
        std::vector<const Model *> modelsToInit;

        for (const Model &model : models)
        {
            bool modelBufferFound = false;
            for (ModelBuffer &modelBuffer : modelBuffers)
            {
                if (model.getHandle() == modelBuffer.handle)
                {
                    modelBufferFound = true;
                    if (model.getVersion() > modelBuffer.version)
                    {
                        updateModelBuffer(modelBuffer, model);
                    }
                    break;
                }
            }

            if (!modelBufferFound)
                modelsToInit.push_back(&model);
        }

        if (modelsToInit.size() == 1)
        {
            initModelBuffer(*modelsToInit.front());
        }
        else if (modelsToInit.size() > 1)
        {
            modelMutex.unlock();

            std::vector<std::thread> workers;
            workers.reserve(modelsToInit.size());
            for (const Model *modelPtr : modelsToInit)
            {
                workers.emplace_back([this, modelPtr]
                                     { initModelBuffer(*modelPtr); });
            }

            for (std::thread &w : workers)
                w.join();

            modelMutex.lock();
        }
    }

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

        VkCommandPool threadLocalCommandPool = VK_NULL_HANDLE;
        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = transferQueueFamilyIndex};
        vkCheck(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &threadLocalCommandPool), {'V', 208});

        VkFence uploadFence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &uploadFence), {'V', 216});

        vkCheck(copyBuffer(threadLocalCommandPool, stagingBuffer, meshBuffer.vertexBuffer, bufferSize, uploadFence), {'V', 224});

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        vkDestroyFence(device, uploadFence, nullptr);
        vkDestroyCommandPool(device, threadLocalCommandPool, nullptr);
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

        VkCommandPool threadLocalCommandPool = VK_NULL_HANDLE;
        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = transferQueueFamilyIndex};
        vkCheck(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &threadLocalCommandPool), {'V', 208});

        VkFence uploadFence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &uploadFence), {'V', 216});

        vkCheck(copyBuffer(threadLocalCommandPool, stagingBuffer, meshBuffer.indexBuffer, bufferSize, uploadFence), {'V', 224});

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

        vkDestroyFence(device, uploadFence, nullptr);
        vkDestroyCommandPool(device, threadLocalCommandPool, nullptr);
    }

    void Renderer::initModelBuffer(const Model &model)
    {
        ModelBuffer newModelBuffer(model.getHandle());

        newModelBuffer.version = model.getVersion();

        for (const Mesh &mesh : model.getMeshes())
        {
            MeshBuffer newMeshBuffer;

            const std::vector<Vertex> &vertices = mesh.getVertices();
            const std::vector<uint32_t> &indices = mesh.getIndices();

            if(!vertices.empty() && vertices[0].col.a < 1.0f)
                newMeshBuffer.isTransparent = true;

            newMeshBuffer.vertexCount = vertices.size();
            newMeshBuffer.indexCount = indices.size();
            createVertexBuffer(newMeshBuffer, vertices);
            createIndexBuffer(newMeshBuffer, indices);

            if (!mesh.getTextureFilePath().empty())
                newMeshBuffer.texIndex = createTexture(mesh.getTextureFilePath());

            newModelBuffer.meshBuffers.push_back(newMeshBuffer);
        }

        {
            std::lock_guard<std::recursive_mutex> lock(modelMutex);
            modelBuffers.push_back(newModelBuffer);
        }
    }

    void Renderer::updateModelBuffer(ModelBuffer &modelBuffer, const Model &model)
    {
        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            vkCheck(vkDeviceWaitIdle(device), {'V', 235});
        }

        for (MeshBuffer &meshBuffer : modelBuffer.meshBuffers)
            destroyMeshBuffer(meshBuffer);

        modelBuffer.meshBuffers.clear();

        for (const Mesh &mesh : model.getMeshes())
        {
            MeshBuffer newMeshBuffer;
            newMeshBuffer.vertexCount = mesh.getVertices().size();
            newMeshBuffer.indexCount = mesh.getIndices().size();
            createVertexBuffer(newMeshBuffer, mesh.getVertices());
            createIndexBuffer(newMeshBuffer, mesh.getIndices());
            modelBuffer.meshBuffers.push_back(newMeshBuffer);
        }

        modelBuffer.version = model.getVersion();
    }

    void Renderer::removeOrphanedModel(const std::vector<ModelInstance> &modelInstances)
    {
        vkDeviceWaitIdle(device);
        for (std::vector<ModelBuffer>::iterator it = modelBuffers.begin(); it != modelBuffers.end();)
        {
            bool hasInstance = false;
            for (const ModelInstance &instance : modelInstances)
            {
                if (instance.modelHandle == it->handle)
                {
                    hasInstance = true;
                    break;
                }
            }

            if (!hasInstance)
            {
                for (MeshBuffer &meshBuffer : it->meshBuffers)
                    destroyMeshBuffer(meshBuffer);

                it = modelBuffers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}