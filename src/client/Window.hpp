// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

#include <GLFW/glfw3.h>

namespace VE
{
    class Window
    {
    public:
        Window(glm::uvec2 size, std::string name);

        [[nodiscard]] GLFWwindow *ptr() const;

        [[nodiscard]] glm::uvec2 getSize() const;

        [[nodiscard]] bool isOpen() const;

        [[nodiscard]] float getAspectRatio() const;

        ~Window();

    private:
        GLFWwindow *window;
        glm::uvec2 size = {};
    };

}