// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "../../shared/log.hpp"

namespace VE
{

    void Renderer::syncWidgetBuffers(const std::vector<Widget> &widgets)
    {
        std::vector<const Widget *> widgetsToInit;

        for (const Widget &widget : widgets)
        {
            bool widgetBufferFound = false;
            for (WidgetBuffer &widgetBuffer : widgetBuffers)
            {
                if (widget.getHandle() == widgetBuffer.handle)
                {
                    widgetBufferFound = true;
                    if (widget.getVersion() > widgetBuffer.version)
                        updateWidgetBuffer(widgetBuffer, widget);

                    break;
                }
            }

            if (!widgetBufferFound)
                widgetsToInit.push_back(&widget);
        }

        if (!widgetsToInit.empty())
        {
            std::vector<WidgetBuffer> results(widgetsToInit.size());
            for (size_t i = 0; i < widgetsToInit.size(); i++)
                uploadThreadPool.submit([this, &results, &widgetsToInit, i](uint32_t workerIndex)
                                  { results[i] = createWidgetBuffer(*widgetsToInit[i], workerIndex); });
            uploadThreadPool.wait();

            for (WidgetBuffer &widgetBuffer : results)
                widgetBuffers.push_back(std::move(widgetBuffer));
        }
    }

    WidgetBuffer Renderer::createWidgetBuffer(const Widget &widget, uint32_t uploadContextIndex)
    {
        WidgetBuffer newWidgetBuffer(widget.getHandle());

        newWidgetBuffer.version = widget.getVersion();
        newWidgetBuffer.materials = widget.getMaterials();

        for (const Mesh &mesh : widget.getMeshes())
        {
            MeshBuffer meshBuffer = createMeshBuffer(mesh, newWidgetBuffer.materials[mesh.getMaterialIndex()].baseColor.a < 1.f, uploadContextIndex);

            newWidgetBuffer.meshBuffers.push_back(meshBuffer);

            Material &material = newWidgetBuffer.materials[meshBuffer.materialIndex];

            if (!material.texturePixels.empty())
                material.texIndex = createTexture(material.texturePixels, material.textureSize, uploadContextIndex);
        }

        return newWidgetBuffer;
    }

    void Renderer::updateWidgetBuffer(WidgetBuffer &widgetBuffer, const Widget &widget)
    {
        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            vkCheck(vkDeviceWaitIdle(device), {'R', 235});
        }

        for (MeshBuffer &meshBuffer : widgetBuffer.meshBuffers)
        {
            destroyMeshBuffer(meshBuffer);

            if (widgetBuffer.materials[meshBuffer.materialIndex].texIndex != INVALID_TEXTURE_INDEX)
                destroyImageAttachment(textures.attachments[widgetBuffer.materials[meshBuffer.materialIndex].texIndex]);
        }

        widgetBuffer.meshBuffers.clear();

        widgetBuffer.materials = widget.getMaterials();

        for (const Mesh &mesh : widget.getMeshes())
        {
            MeshBuffer meshBuffer = createMeshBuffer(mesh, widgetBuffer.materials[mesh.getMaterialIndex()].baseColor.a < 1.f, 0);

            widgetBuffer.meshBuffers.push_back(meshBuffer);

            Material &material = widgetBuffer.materials[meshBuffer.materialIndex];

            if (!material.texturePixels.empty())
                material.texIndex = createTexture(material.texturePixels, material.textureSize, 0);
        }

        widgetBuffer.version = widget.getVersion();
    }

}