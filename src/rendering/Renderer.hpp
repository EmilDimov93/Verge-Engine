// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../system/WindowManager.hpp"
#include "../system/Input.hpp"
#include "../system/FPSManager.hpp"

#include "VulkanManager.hpp"

#include "../Log.hpp"

struct VE_STRUCT_RENDERER_CREATE_INFO
{
    std::string projectName;
    Size2 windowSize = {};
    LogOutputMode logOutputMode = VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE;
    uint16_t targetFps = VE_DEFAULT_FPS;
};

class Renderer
{
public:
    WindowManager window;
    VulkanManager vulkan;
    FpsManager fps;

    Renderer(const VE_STRUCT_RENDERER_CREATE_INFO& info = {});

    bool tick(DrawData drawData);

    ~Renderer();

private:
};