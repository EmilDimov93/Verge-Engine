// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

#include <chrono>

#define VE_DEFAULT_FPS 140

class FpsManager
{
public:
    FpsManager();
    void sync();
    void setTarget(uint16_t targetFps);
    uint16_t getFps() const;
    double getFrameTime() const;

private:
    std::chrono::steady_clock::time_point timeAtStartOfFrame;
    ve_time_t targetFrameTime = 0;

    ve_time_t lastFrameTime = 0;
    uint16_t currentFps = 0;
};