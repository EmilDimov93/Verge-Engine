// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WindowManager.hpp"

#include "../Log.hpp"

#include <GLFW/glfw3.h>

WindowManager::WindowManager()
{
    if (!glfwInit())
        Log::add('G', 200);
    isInitialized = true;

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

GLFWwindow* WindowManager::getReference()
{
    return window;
}

Size2 WindowManager::getSize()
{
    return windowSize;
}

bool WindowManager::isOpen(){
    return !glfwWindowShouldClose(window);
}

float WindowManager::getAspectRatio()
{
    if(!isInitialized){
        Log::add('G', 100);
        return 1.0f;
    }
    if(windowSize.h == 0)
        return 1.0f;

    return (float)windowSize.w / (float)windowSize.h;
}

WindowManager::~WindowManager(){
    if (window){
        glfwDestroyWindow(window);
        window = nullptr;
    }

    if(isInitialized)
        glfwTerminate();
    isInitialized = false;
}