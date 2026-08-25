// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "UI.hpp"

#include "../shared/HandleFactory.hpp"
#include "../shared/ModelLoader.hpp"

#include "../shared/Log.hpp"

#include <array>

namespace VE
{
    void UI::tick(bool mouseBtnClicked, glm::vec2 mousePos, glm::uvec2 windowSize)
    {
        if (!mouseBtnClicked)
            return;

        for (const auto &[handle, callback] : callbacks)
            if (checkCursorCollision(handle, mousePos, windowSize))
                callback();
    }

    UIDrawData UI::getWidgetData() const
    {
        UIDrawData drawData(widgets, widgetInstances);
        return drawData;
    }

    WidgetHandle UI::addWidget(const std::string &filePath)
    {
        std::pair<std::vector<Mesh>, std::vector<Material>> data = loadModel(filePath);

        if (data.first.empty())
        {
            Log::add('E', 102);
            return WidgetHandle::INVALID;
        }

        WidgetHandle newWidgetHandle = HandleFactory<WidgetHandle>::getNewHandle();

        widgets.emplace_back(newWidgetHandle, data.first, data.second);

        return newWidgetHandle;
    }

    WidgetInstanceHandle UI::addWidgetInstance(WidgetHandle widgetHandle, glm::vec2 coords, float uniformScale, const std::function<void()> &callback)
    {
        if (widgetHandle == WidgetHandle::INVALID)
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

        widgetInstances.emplace_back(newHandle, widgetHandle, coords, uniformScale);

        if (callback)
            callbacks.insert(std::make_pair(newHandle, callback));

        return widgetInstances.back().handle;
    }

    void UI::setCallback(WidgetInstanceHandle handle, const std::function<void()> &callback)
    {
        auto it = callbacks.find(handle);
        if (it != callbacks.end())
            it->second = std::move(callback);
        else
            callbacks.emplace(handle, std::move(callback));
    }

    bool UI::checkCursorCollision(WidgetInstanceHandle handle, glm::vec2 mousePos, glm::uvec2 windowSize) const
    {
        const WidgetInstance *foundInstance = nullptr;
        for (const WidgetInstance &instance : widgetInstances)
        {
            if (instance.handle == handle)
            {
                foundInstance = &instance;
                break;
            }
        }

        if(!foundInstance)
            return false;

        const Widget *foundWidget = nullptr;
        for (const Widget &widget : widgets)
        {
            if (widget.getHandle() == foundInstance->widgetHandle)
            {
                foundWidget = &widget;
                break;
            }
        }

        if (!foundWidget)
            return false;

        glm::mat4 model = Transform(glm::vec3((foundInstance->coords.x + 1) / 2 * windowSize.x, (foundInstance->coords.y + 1) / 2 * windowSize.y, 0.f), glm::vec3(), glm::vec3(foundInstance->uniformScale)).toMat();

        for (const Mesh &mesh : foundWidget->getMeshes())
        {
            const std::vector<Vertex> &vertices = mesh.getVertices();
            const std::vector<uint32_t> &indices = mesh.getIndices();
            uint32_t counter = 0;

            std::array<glm::vec2, 3> triangle;

            for (const uint32_t &index : indices)
            {
                triangle[counter] = glm::vec2(model * glm::vec4(vertices[index].pos.x, vertices[index].pos.y, 0.f, 1.f));

                if (counter == 2)
                {
                    auto edgeSign = [](glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
                    {
                        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
                    };

                    float d1 = edgeSign(mousePos, triangle[0], triangle[1]);
                    float d2 = edgeSign(mousePos, triangle[1], triangle[2]);
                    float d3 = edgeSign(mousePos, triangle[2], triangle[0]);

                    bool has_neg = (d1 < 0.f) || (d2 < 0.f) || (d3 < 0.f);
                    bool has_pos = (d1 > 0.f) || (d2 > 0.f) || (d3 > 0.f);

                    if (!(has_neg && has_pos))
                        return true;

                    counter = 0;
                    continue;
                }

                counter++;
            }
        }

        return false;
    }
}
