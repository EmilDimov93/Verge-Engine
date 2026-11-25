// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "../../shared/log.hpp"

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

        if (!modelsToInit.empty())
        {
            std::vector<ModelBuffer> results(modelsToInit.size());
            for (size_t i = 0; i < modelsToInit.size(); ++i)
                threadPool.submit([this, &results, &modelsToInit, i](uint32_t workerIndex)
                                  { results[i] = createModelBuffer(*modelsToInit[i]); });
            threadPool.wait();

            for (ModelBuffer &modelBuffer : results)
                modelBuffers.push_back(std::move(modelBuffer));
        }
    }

    ModelBuffer Renderer::createModelBuffer(const Model &model)
    {
        ModelBuffer newModelBuffer(model.getHandle());

        newModelBuffer.version = model.getVersion();

        newModelBuffer.materials = model.getMaterials();

        for (const Mesh &mesh : model.getMeshes())
        {
            MeshBuffer meshBuffer = createMeshBuffer(mesh, newModelBuffer.materials[mesh.getMaterialIndex()].baseColor.a < 1.f);

            newModelBuffer.meshBuffers.push_back(meshBuffer);

            Material &material = newModelBuffer.materials[meshBuffer.materialIndex];

            if (!material.texturePixels.empty())
                material.texIndex = createTexture(material.texturePixels, material.textureSize);
        }

        return newModelBuffer;
    }

    void Renderer::updateModelBuffer(ModelBuffer &modelBuffer, const Model &model)
    {
        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            vkCheck(vkDeviceWaitIdle(device), {'R', 235});
        }

        for (MeshBuffer &meshBuffer : modelBuffer.meshBuffers)
        {
            destroyMeshBuffer(meshBuffer);

            if (modelBuffer.materials[meshBuffer.materialIndex].texIndex != INVALID_TEXTURE_INDEX)
                destroyImageAttachment(textures.attachments[modelBuffer.materials[meshBuffer.materialIndex].texIndex]);
        }

        modelBuffer.meshBuffers.clear();

        modelBuffer.materials = model.getMaterials();

        for (const Mesh &mesh : model.getMeshes())
        {
            MeshBuffer meshBuffer = createMeshBuffer(mesh, modelBuffer.materials[mesh.getMaterialIndex()].baseColor.a < 1.f);

            modelBuffer.meshBuffers.push_back(meshBuffer);

            Material &material = modelBuffer.materials[meshBuffer.materialIndex];

            if (!material.texturePixels.empty())
                material.texIndex = createTexture(material.texturePixels, material.textureSize);
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
                {
                    destroyMeshBuffer(meshBuffer);

                    if (it->materials[meshBuffer.materialIndex].texIndex != INVALID_TEXTURE_INDEX)
                        destroyImageAttachment(textures.attachments[it->materials[meshBuffer.materialIndex].texIndex]);
                }

                it = modelBuffers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}