// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

#include <GLFW/glfw3.h>

namespace VE
{

    class WindowManager
    {
    public:
        WindowManager(Size2 size, std::string name);

        [[nodiscard]] GLFWwindow *getReference() const;

        [[nodiscard]] Size2 getSize() const;

        [[nodiscard]] bool isOpen() const;

        [[nodiscard]] float getAspectRatio() const;

        ~WindowManager();

    private:
        GLFWwindow *window;
        Size2 size = {};
    };

}