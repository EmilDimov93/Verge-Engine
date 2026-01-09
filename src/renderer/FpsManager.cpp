// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "FPSManager.hpp"

#include <thread>

FpsManager::FpsManager()
{
    setTarget(VE_DEFAULT_FPS);
    timeAtStartOfFrame = std::chrono::steady_clock::now();
}

void FpsManager::sync()
{
    using namespace std::chrono;
    using std::this_thread::sleep_for;

    auto now = steady_clock::now();
    auto elapsed = now - timeAtStartOfFrame;
    auto target = duration<ve_time_t>(targetFrameTime);

    if (elapsed < target)
    {
        auto remaining = target - elapsed;
        if (remaining > milliseconds(1))
        {
            sleep_for(remaining - milliseconds(1));
        }
    }

    while ((steady_clock::now() - timeAtStartOfFrame) < target)
    {
        std::this_thread::yield();
    }

    now = steady_clock::now();
    currentFps = static_cast<uint16_t>(1.0 / duration<ve_time_t>(now - timeAtStartOfFrame).count());
    lastFrameTime = std::chrono::duration<ve_time_t>(now - timeAtStartOfFrame).count();
    timeAtStartOfFrame = now;
}

void FpsManager::setTarget(uint16_t targetFps)
{
    if (targetFps > 0)
    {
        targetFrameTime = 1.0 / targetFps;
    }
}

uint16_t FpsManager::getFps() const
{
    return currentFps;
}

ve_time_t FpsManager::getFrameTime() const
{
    return lastFrameTime;
}