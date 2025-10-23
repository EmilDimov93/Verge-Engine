#include "FPSManager.h"

#include <iostream>

void FPSManager::CorrectFrameTime()
{
    while (true)
    {
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - timeAtStartOfFrame;
        if (elapsed.count() >= targetFrameTime)
        {
            break;
        }
    }

    currentFPS = static_cast<int>(1 / (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timeAtStartOfFrame).count()));

    timeAtStartOfFrame = std::chrono::high_resolution_clock::now();
}

void FPSManager::SetTargetFPS(int targetFPS)
{
    targetFrameTime = 1.0 / targetFPS;
}

int FPSManager::GetFPS()
{
    return currentFPS;
}