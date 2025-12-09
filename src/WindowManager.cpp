// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WindowManager.hpp"

#include <GLFW/glfw3.h>

#include "Log.hpp"

WindowManager::WindowManager()
{
    if (!glfwInit())
        Log::add('G', 200);
    isGlfwInitialized = true;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    windowSize.w = mode->width / 2;
    windowSize.h = mode->height / 2;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(windowSize.w, windowSize.h, "Verge Engine", nullptr, nullptr);
    if (!window)
    {
        Log::add('G', 201);
    }

    Log::add('G', 000);
}

GLFWwindow* WindowManager::getWindowReference()
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
    if (window){
        glfwDestroyWindow(window);
        window = nullptr;
    }

    if(isGlfwInitialized)
        glfwTerminate();
    isGlfwInitialized = false;
}