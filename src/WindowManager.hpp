// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

class LogManager;

struct GLFWwindow;

class WindowManager
{
public:

    void init(LogManager *logRef);

    GLFWwindow *getWindowReference();

    Size getWindowSize();

    void cleanup();

    bool shouldNotClose();

private:
    GLFWwindow *window;
    Size windowSize = {0, 0};

    LogManager *log;
};