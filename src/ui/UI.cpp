// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "UI.hpp"

#include "../shared/HandleFactory.hpp"

#include "../shared/Log.hpp"

#include <array>

namespace VE
{
    void UI::tick(bool mouseBtnClicked, Position2 mousePos)
    {
        if (!mouseBtnClicked)
            return;

        for (const auto &[handle, callback] : callbacks)
            if(checkCursorCollision(handle, mousePos))
                callback();
    }

    UIDrawData UI::getWidgetData() const
    {
        UIDrawData drawData(widgets, widgetInstances);
        return drawData;
    }

    WidgetHandle UI::addWidget(const std::string &filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();

        if (ext != ".obj")
        {
            Log::add('E', 101);
            return INVALID_WIDGET_HANDLE;
        }

        WidgetHandle newWidgetHandle = HandleFactory<WidgetHandle>::getNewHandle();

        std::vector<Mesh> meshes = loadOBJ(filePath);

        if (meshes.empty())
        {
            Log::add('E', 102);
            return INVALID_WIDGET_HANDLE;
        }

        widgets.emplace_back(newWidgetHandle, meshes);

        return newWidgetHandle;
    }

    WidgetInstanceHandle UI::addWidgetInstance(WidgetHandle widgetHandle, glm::vec2 coords, const std::function<void()>& callback)
    {
        if (widgetHandle == INVALID_WIDGET_HANDLE)
            Log::add('W', 200);

        bool foundWidget = false;
        for (const Widget &widget : widgets)
        {
            if (widget.getHandle() == widgetHandle)
            {
                foundWidget = true;
                break;
            }
        }

        if (!foundWidget)
            Log::add('W', 200);

        WidgetInstanceHandle newHandle = HandleFactory<WidgetInstanceHandle>::getNewHandle();

        widgetInstances.emplace_back(newHandle, widgetHandle, glm::translate(glm::mat4(1.f), glm::vec3(coords.x, coords.y, 0.0f)));

        if(callback)
            callbacks.insert(std::make_pair(newHandle, callback));

        return widgetInstances.back().handle;
    }

    void UI::setCallback(WidgetInstanceHandle handle, const std::function<void()>& callback)
    {
        auto it = callbacks.find(handle);
        if (it != callbacks.end())
            it->second = std::move(callback);
        else
            callbacks.emplace(handle, std::move(callback));
    }

    bool UI::checkCursorCollision(WidgetInstanceHandle handle, Position2 mousePos) const
    {
        for (const WidgetInstance &instance : widgetInstances)
        {
            if (instance.handle == handle)
            {
                for (const Widget &widget : widgets)
                {
                    if (widget.getHandle() == instance.widgetHandle)
                    {
                        for (const Mesh &mesh : widget.getMeshes())
                        {
                            const std::vector<Vertex> &vertices = mesh.getVertices();
                            const std::vector<uint32_t> &indices = mesh.getIndices();
                            uint32_t counter = 0;

                            std::array<glm::vec2, 3> triangle;

                            for (const uint32_t &index : indices)
                            {
                                triangle[counter] = glm::vec2(instance.modelMat * glm::vec4(vertices[index].pos.x, vertices[index].pos.y, 0.0f, 1.0f));

                                if (counter == 2)
                                {
                                    auto edgeSign = [](glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
                                    {
                                        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
                                    };

                                    float d1 = edgeSign({mousePos.x, mousePos.y}, triangle[0], triangle[1]);
                                    float d2 = edgeSign({mousePos.x, mousePos.y}, triangle[1], triangle[2]);
                                    float d3 = edgeSign({mousePos.x, mousePos.y}, triangle[2], triangle[0]);

                                    bool has_neg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
                                    bool has_pos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);

                                    if(!(has_neg && has_pos))
                                        return true;

                                    counter = 0;
                                    continue;
                                }

                                counter++;
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }

        return false;
    }
}
