// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/MeshLoader.hpp"
#include "../shared/DrawData.hpp"

#include <unordered_map>

namespace VE
{
    class UI
    {
    public:
        void tick(bool mouseBtnClicked, Position2 mousePos, Size2 windowSize);

        UIDrawData getWidgetData() const;

        WidgetHandle addWidget(const std::string &filePath);

        WidgetInstanceHandle addWidgetInstance(WidgetHandle handle, glm::vec2 coords, float uniformScale = 1.f, const std::function<void()>& callback = nullptr);

        void setCallback(WidgetInstanceHandle handle, const std::function<void()>& callback);

    private:
        std::vector<Widget> widgets;
        std::vector<WidgetInstance> widgetInstances;

        std::unordered_map<WidgetInstanceHandle, std::function<void()>> callbacks;

        bool checkCursorCollision(WidgetInstanceHandle handle, Position2 mousePos, Size2 windowSize) const;
    };
}