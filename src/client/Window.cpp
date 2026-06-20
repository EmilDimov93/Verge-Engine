// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Window.hpp"

#include "../shared/Log.hpp"

namespace VE
{
    Window::Window(Size2 size, std::string name)
    {
        if (!glfwInit())
            Log::add('G', 200);

        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        if (size.w <= mode->width && size.h <= mode->height)
        {
            this->size = size;
        }
        else
        {
            Log::add('G', 101);

            this->size = {static_cast<uint32_t>(mode->width / 2), static_cast<uint32_t>(mode->height / 2)};
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(this->size.w, this->size.h, name.c_str(), nullptr, nullptr);
        if (!window)
            Log::add('G', 201);

        glfwSetWindowUserPointer(window, this);

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
            Window *w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            w->size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        });

        Log::add('G', 000);
    }

    GLFWwindow *Window::ptr() const
    {
        return window;
    }

    Size2 Window::getSize() const
    {
        return size;
    }

    bool Window::isOpen() const
    {
        return !glfwWindowShouldClose(window);
    }

    float Window::getAspectRatio() const
    {
        if (size.h == 0)
            return 1.0f;

        return static_cast<float>(size.w) / static_cast<float>(size.h);
    }

    Window::~Window()
    {
        if (window)
            glfwDestroyWindow(window);
        window = nullptr;

        glfwTerminate();
    }
}