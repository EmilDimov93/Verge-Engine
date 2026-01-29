// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../shared/Input.hpp"
#include "../shared/Log.hpp"

Renderer::Renderer(const VE_STRUCT_RENDERER_CREATE_INFO &info) : window(info.windowSize, info.projectName), vulkan(window.getReference(), window.getSize())
{
    Input::init(window.getReference());
    Log::init(info.logOutputMode);
    fps.setTarget(info.targetFps);
}

bool Renderer::tick(DrawData drawData, AudioData audioData)
{
    fps.sync();

    Input::refresh();

    vulkan.drawFrame(drawData);

    audio.tick(audioData);

    return window.isOpen();
}

Renderer::~Renderer()
{
    Log::end();
}

ve_time_t Renderer::getFrameTime() const
{
    return fps.getFrameTime();
}

uint32_t Renderer::getFps() const
{
    return fps.getFps();
}

void Renderer::setTargetFps(uint16_t target)
{
    fps.setTarget(target);
}

float Renderer::getAspectRatio()
{
    return window.getAspectRatio();
}
