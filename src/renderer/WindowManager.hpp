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

    GLFWwindow *getReference() const;

    Size2 getSize() const;

    bool isOpen() const;

    float getAspectRatio() const;

    ~WindowManager();

private:
    GLFWwindow *window;
    Size2 size = {};

    bool isInitialized = false;
};