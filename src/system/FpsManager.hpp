// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../definitions.hpp"

#include <chrono>

#define VE_DEFAULT_FPS 140

class FpsManager
{
public:
    FpsManager();
    void sync();
    void setTarget(uint16_t targetFps);
    uint16_t getFps();
    double getFrameTime();

private:
    std::chrono::steady_clock::time_point timeAtStartOfFrame;
    ve_time targetFrameTime = 0;

    ve_time lastFrameTime = 0;
    uint16_t currentFps = 0;
};