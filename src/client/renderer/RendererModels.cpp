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

    void Renderer::initModelBuffer(const Model &model)
    {
        ModelBuffer newModelBuffer(model.getHandle());

        newModelBuffer.version = model.getVersion();

        newModelBuffer.materials = model.getMaterials();

        for (const Mesh &mesh : model.getMeshes())
        {
            MeshBuffer meshBuffer = createMeshBuffer(mesh, newModelBuffer.materials[mesh.getMaterialIndex()].baseColor.a < 1.0f);

            newModelBuffer.meshBuffers.push_back(meshBuffer);

            Material &material = newModelBuffer.materials[meshBuffer.materialIndex];

            if (!material.texturePixels.empty())
                material.texIndex = createTexture(material.texturePixels, material.textureSize);
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
            MeshBuffer meshBuffer = createMeshBuffer(mesh, modelBuffer.materials[mesh.getMaterialIndex()].baseColor.a < 1.0f);

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

    bool Renderer::syncShadowMaps(const std::vector<ModelInstance> &instances)
    {
        bool hasChanged = false;

        for (size_t i = 1; i < shadowMapCount; i++)
        {
            bool hasInstance = false;
            for (const ModelInstance &instance : instances)
            {
                if (shadowMaps[i].instanceHandle == instance.handle)
                {
                    hasInstance = true;
                    break;
                }
            }

            if (!hasInstance)
            {
                vkDeviceWaitIdle(device);

                destroyImageAttachment(shadowMaps[i].depthAttachment);

                shadowMapCount--;

                for (size_t j = i; j < shadowMapCount; j++)
                {
                    shadowMaps[j] = shadowMaps[j + 1];
                }

                i--;

                hasChanged = true;
            }
        }

        for (const ModelInstance &instance : instances)
        {
            if (instance.lightIntensity > 0.f)
            {
                bool hasShadowMap = false;
                for (size_t i = 1; i < shadowMapCount; i++)
                {
                    if (shadowMaps[i].instanceHandle == instance.handle)
                    {
                        hasShadowMap = true;
                        break;
                    }
                }

                if (!hasShadowMap)
                {
                    if (shadowMapCount >= MAX_SHADOW_MAPS)
                        Log::add('R', 228);

                    shadowMaps[shadowMapCount] = createShadowMap(instance.handle);
                    shadowMapCount++;

                    hasChanged = true;
                }
            }
        }

        return hasChanged;
    }
}