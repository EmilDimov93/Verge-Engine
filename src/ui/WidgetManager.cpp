// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WidgetManager.hpp"

#include "../shared/HandleFactory.hpp"

#include "../shared/Log.hpp"

namespace VE
{
    UIDrawData WidgetManager::getWidgetData() const
    {
        UIDrawData drawData(widgets, widgetInstances);
        return drawData;
    }

    WidgetHandle WidgetManager::addWidget(const std::string &filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();

        if (ext != ".obj")
        {
            Log::add('E', 101);
            return INVALID_WIDGET_HANDLE;
        }

        WidgetHandle newWidgetHandle = HandleFactory<WidgetHandle>::getNewHandle();

        std::vector<Mesh> meshes = loadOBJ(filePath);

        if(meshes.empty())
        {
            Log::add('E', 102);
            return INVALID_WIDGET_HANDLE;
        }

        widgets.emplace_back(newWidgetHandle, meshes);

        return newWidgetHandle;
    }

    WidgetInstanceHandle WidgetManager::addWidgetInstance(WidgetHandle widgetHandle)
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

        widgetInstances.emplace_back(HandleFactory<WidgetInstanceHandle>::getNewHandle(), widgetHandle, glm::mat4(1.0f));

        return widgetInstances.back().handle;
    }
}
