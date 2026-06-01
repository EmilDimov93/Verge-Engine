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
                    {
                        Log::add('E', 100);
                        // updateWidgetBuffer(widgetBuffer, widget);
                    }
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

        for (const Mesh &mesh : widget.getMeshes())
        {
            MeshBuffer newMeshBuffer;

            newMeshBuffer.vertexCount = mesh.getVertices().size();
            newMeshBuffer.indexCount = mesh.getIndices().size();
            createVertexBuffer(newMeshBuffer, mesh.getVertices());
            createIndexBuffer(newMeshBuffer, mesh.getIndices());

            if (!mesh.getTextureFilePath().empty())
                newMeshBuffer.texIndex = createTexture(mesh.getTextureFilePath());

            newWidgetBuffer.meshBuffers.push_back(newMeshBuffer);
        }

        {
            std::lock_guard<std::recursive_mutex> lock(widgetMutex);
            widgetBuffers.push_back(newWidgetBuffer);
        }
    }

}