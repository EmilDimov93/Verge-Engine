// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WindowManager.hpp"
#include "VulkanManager.hpp"
#include "FPSManager.hpp"

#include "../shared/Log.hpp"

struct VE_STRUCT_RENDERER_CREATE_INFO
{
    std::string projectName = "Verge Engine Program";
    Size2 windowSize = {};
    LogOutputMode logOutputMode = VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE;
    uint16_t targetFps = VE_DEFAULT_FPS;
};

class Renderer
{
public:
    Renderer(const VE_STRUCT_RENDERER_CREATE_INFO &info = {});

    bool tick(DrawData drawData);

    ~Renderer();

    ve_time_t getFrameTime() const;
    uint32_t getFps() const;
    void setTargetFps(uint16_t target);

    float getAspectRatio();

private:
    WindowManager window;
    VulkanManager vulkan;
    FpsManager fps;
};