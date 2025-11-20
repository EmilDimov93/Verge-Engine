// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WindowManager.hpp"

#include <GLFW/glfw3.h>

#include "LogManager.hpp"

WindowManager::WindowManager(LogManager *logRef)
{
    log = logRef;

    if (!glfwInit())
    {
        log->add('G', 200);
    }

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    windowSize.w = mode->width / 2;
    windowSize.h = mode->height / 2;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(windowSize.w, windowSize.h, "Verge Engine", nullptr, nullptr);
    if (!window)
    {
        log->add('G', 201);
    }

    log->add('G', 000);
}

GLFWwindow *WindowManager::getWindowReference()
{
    return window;
}

Size2 WindowManager::getWindowSize()
{
    return windowSize;
}

bool WindowManager::isOpen(){
    return !glfwWindowShouldClose(window);
}

WindowManager::~WindowManager(){
    glfwDestroyWindow(window);
    glfwTerminate();
}