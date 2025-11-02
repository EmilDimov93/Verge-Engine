// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <chrono>

class FpsManager
{
private:
    std::chrono::steady_clock::time_point timeAtStartOfFrame;
    double targetFrameTime = 0;
    uint16_t currentFps = 0;

public:
    void syncFrameTime();
    void setTargetFps(uint16_t targetFps);
    uint16_t getFps();
};