// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

class Log;

struct GLFWwindow;

class WindowManager
{
public:
    WindowManager(Size2 size, std::string projectName);

    GLFWwindow *getReference();

    Size2 getSize();

    bool isOpen();

    float getAspectRatio();

    ~WindowManager();

private:
    GLFWwindow *window;
    Size2 size = {};

    bool isInitialized = false;
};