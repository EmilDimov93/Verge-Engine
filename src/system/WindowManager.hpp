// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../definitions.hpp"

class Log;

struct GLFWwindow;

class WindowManager
{
public:

    WindowManager();

    GLFWwindow* getReference();

    Size2 getSize();

    bool isOpen();

    float getAspectRatio();

    ~WindowManager();

private:
    GLFWwindow* window;
    Size2 windowSize = {0, 0};

    bool isInitialized = false;
};