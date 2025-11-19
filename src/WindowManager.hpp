// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "LogManager.hpp"

#include <GLFW/glfw3.h>

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