// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include <GLFW/glfw3.h>

#include "LogManager.hpp"
#include "definitions.hpp"

class GlfwManager
{
private:
    LogManager *log;

public:
    void initWindow(GLFWwindow **window, Size windowSize, LogManager *logRef);
};