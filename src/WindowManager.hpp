// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

class LogManager;

struct GLFWwindow;

class WindowManager
{
public:

    WindowManager(LogManager *logRef);

    GLFWwindow *getWindowReference();

    Size getWindowSize();

    bool isOpen();

    ~WindowManager();

private:
    GLFWwindow *window;
    Size windowSize = {0, 0};

    LogManager *log;
};