#include "FPSManager.h"
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
    currentFps = static_cast<int>(1.0 / std::chrono::duration<double>(now - timeAtStartOfFrame).count());
    timeAtStartOfFrame = now;
}

void FpsManager::setTargetFps(int targetFps)
{
    targetFrameTime = 1.0 / targetFps;
}

int FpsManager::getFps()
{
    return currentFps;
}