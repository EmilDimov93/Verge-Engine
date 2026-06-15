// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../../shared/Log.hpp"

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

        if (widgetsToInit.size() == 1)
        {
            initWidgetBuffer(*widgetsToInit.front());
        }
        else if (widgetsToInit.size() > 1)
        {
            widgetMutex.unlock();

            std::vector<std::thread> workers;
            workers.reserve(widgetsToInit.size());
            for (const Widget *widgetPtr : widgetsToInit)
            {
                workers.emplace_back([this, widgetPtr]
                                     { initWidgetBuffer(*widgetPtr); });
            }

            for (std::thread &w : workers)
                w.join();

            widgetMutex.lock();
        }
    }

    void Renderer::initWidgetBuffer(const Widget &widget)
    {
        WidgetBuffer newWidgetBuffer(widget.getHandle());

        newWidgetBuffer.version = widget.getVersion();
        newWidgetBuffer.materials = widget.getMaterials();

        for (const Mesh &mesh : widget.getMeshes())
        {
            MeshBuffer newMeshBuffer;

            newMeshBuffer.materialIndex = mesh.getMaterialIndex();
            newMeshBuffer.vertexCount = mesh.getVertices().size();
            newMeshBuffer.indexCount = mesh.getIndices().size();
            createVertexBuffer(newMeshBuffer, mesh.getVertices());
            createIndexBuffer(newMeshBuffer, mesh.getIndices());

            if (!newWidgetBuffer.materials[newMeshBuffer.materialIndex].textureFilePath.empty())
                newWidgetBuffer.materials[newMeshBuffer.materialIndex].texIndex = createTexture(newWidgetBuffer.materials[newMeshBuffer.materialIndex].textureFilePath);

            newWidgetBuffer.meshBuffers.push_back(newMeshBuffer);
        }

        {
            std::lock_guard<std::recursive_mutex> lock(widgetMutex);
            widgetBuffers.push_back(newWidgetBuffer);
        }
    }

    void Renderer::updateWidgetBuffer(WidgetBuffer &widgetBuffer, const Widget &widget)
    {
        {
            std::lock_guard<std::mutex> lock(graphicsQueueMutex);
            vkCheck(vkDeviceWaitIdle(device), {'V', 235});
        }

        for (MeshBuffer &meshBuffer : widgetBuffer.meshBuffers)
            destroyMeshBuffer(meshBuffer);

        widgetBuffer.meshBuffers.clear();

        widgetBuffer.materials = widget.getMaterials();

        for (const Mesh &mesh : widget.getMeshes())
        {
            MeshBuffer newMeshBuffer;
            newMeshBuffer.materialIndex = mesh.getMaterialIndex();
            if(widgetBuffer.materials[newMeshBuffer.materialIndex].baseColor.a < 1.0f)
                newMeshBuffer.isTransparent = true;
            newMeshBuffer.vertexCount = mesh.getVertices().size();
            newMeshBuffer.indexCount = mesh.getIndices().size();
            createVertexBuffer(newMeshBuffer, mesh.getVertices());
            createIndexBuffer(newMeshBuffer, mesh.getIndices());
            widgetBuffer.meshBuffers.push_back(newMeshBuffer);
        }

        widgetBuffer.version = widget.getVersion();
    }

}