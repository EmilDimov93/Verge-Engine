// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WindowManager.hpp"

#include <GLFW/glfw3.h>

#include "../shared/Log.hpp"

WindowManager::WindowManager(Size2 size, std::string projectName)
{
    if (!glfwInit())
        Log::add('G', 200);
    isInitialized = true;

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if (size.w > 0 && size.w <= mode->width && size.h > 0 && size.h <= mode->height)
    {
        this->size = size;
    }
    else
    {
        Log::add('G', 102);

        this->size.w = mode->width * 0.5f;
        this->size.h = mode->height * 0.5f;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(this->size.w, this->size.h, projectName.c_str(), nullptr, nullptr);
    if (!window)
    {
        Log::add('G', 201);
    }

    Log::add('G', 000);
}

GLFWwindow *WindowManager::getReference()
{
    return window;
}

Size2 WindowManager::getSize()
{
    return size;
}

bool WindowManager::isOpen()
{
    return !glfwWindowShouldClose(window);
}

float WindowManager::getAspectRatio()
{
    if (!isInitialized)
    {
        Log::add('G', 100);
        return 1.0f;
    }
    if (size.h == 0)
        return 1.0f;

    return (float)size.w / (float)size.h;
}

WindowManager::~WindowManager()
{
    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    if (isInitialized)
        glfwTerminate();
    isInitialized = false;
}