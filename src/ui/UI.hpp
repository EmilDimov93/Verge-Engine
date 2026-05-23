// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/MeshLoader.hpp"
#include "../shared/DrawData.hpp"

namespace VE
{
    class UI
    {
    public:
        UIDrawData getWidgetData() const;

        WidgetHandle addWidget(const std::string &filePath);

        WidgetInstanceHandle addWidgetInstance(WidgetHandle handle, glm::vec2 coords);

    private:
        std::vector<Widget> widgets;
        std::vector<WidgetInstance> widgetInstances;
    };
}