// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../system/WindowManager.hpp"
#include "../system/Input.hpp"
#include "../system/FPSManager.hpp"

#include "VulkanManager.hpp"

class Renderer
{
public:
    WindowManager window;
    VulkanManager vulkan;
    FpsManager fps;

    Renderer();

    bool tick(DrawData drawData);

    ~Renderer();

private:
};