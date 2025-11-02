// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "GlfwManager.hpp"

void GlfwManager::initWindow(GLFWwindow **window, Size windowSize, LogManager *logRef)
{
    log = logRef;

    if (!glfwInit())
    {
        log->add('G', 200);
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    *window = glfwCreateWindow(windowSize.w, windowSize.h, "Verge Engine", nullptr, nullptr);
    if (!*window)
    {
        log->add('G', 201);
        return;
    }

    log->add('G', 000);
}