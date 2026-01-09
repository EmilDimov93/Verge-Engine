// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../shared/Log.hpp"

Renderer::Renderer(const VE_STRUCT_RENDERER_CREATE_INFO &info) : window(info.windowSize, info.projectName), vulkan(window.getReference(), window.getSize())
{
    Input::init(window.getReference());
    Log::init(info.logOutputMode);
    fps.setTarget(info.targetFps);
}

bool Renderer::tick(DrawData drawData)
{
    fps.sync();

    Input::refresh();

    vulkan.drawFrame(drawData);

    return window.isOpen();
}

Renderer::~Renderer()
{
    Log::end();
}
