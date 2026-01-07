// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../Log.hpp"

Renderer::Renderer() : window(50.0f), vulkan(window.getReference(), window.getSize())
{
    Input::init(window.getReference());
    Log::init(VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE);
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
