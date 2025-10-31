// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "FPSManager.hpp"

#include <thread>

void FpsManager::syncFrameTime()
{
    while (true)
    {
        std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - timeAtStartOfFrame;
        if (elapsed.count() >= targetFrameTime)
        {
            break;
        }
    }

    auto now = std::chrono::steady_clock::now();
    currentFps = static_cast<uint16_t>(1.0 / std::chrono::duration<double>(now - timeAtStartOfFrame).count());
    timeAtStartOfFrame = now;
}

void FpsManager::setTargetFps(uint16_t targetFps)
{
    targetFrameTime = 1.0 / targetFps;
}

uint16_t FpsManager::getFps()
{
    return currentFps;
}