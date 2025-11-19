// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <chrono>

#define VE_DEFAULT_FPS 140

class FpsManager
{
public:
    FpsManager(uint16_t targetFps);
    void sync();
    void setTarget(uint16_t targetFps);
    uint16_t getFps();

private:
    std::chrono::steady_clock::time_point timeAtStartOfFrame;
    double targetFrameTime = 0;
    uint16_t currentFps = 0;
};