// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WindowManager.hpp"

#include "../shared/Log.hpp"

namespace VE
{

    WindowManager::WindowManager(Size2 size, std::string name)
    {
        if (!glfwInit())
            Log::add('G', 200);

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

        window = glfwCreateWindow(this->size.w, this->size.h, name.c_str(), nullptr, nullptr);
        if (!window)
            Log::add('G', 201);

        glfwSetWindowUserPointer(window, this);

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
            WindowManager *windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
            windowManager->size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        });

        Log::add('G', 000);
    }

    GLFWwindow *WindowManager::getReference() const
    {
        return window;
    }

    Size2 WindowManager::getSize() const
    {
        return size;
    }

    bool WindowManager::isOpen() const
    {
        return !glfwWindowShouldClose(window);
    }

    float WindowManager::getAspectRatio() const
    {
        if (size.h == 0)
            return 1.0f;

        return static_cast<float>(size.w) / static_cast<float>(size.h);
    }

    WindowManager::~WindowManager()
    {
        if (window)
            glfwDestroyWindow(window);

        window = nullptr;

        glfwTerminate();
    }

}