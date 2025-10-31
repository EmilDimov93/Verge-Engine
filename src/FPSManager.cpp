#include "FPSManager.h"
#include <thread>

void FPSManager::CorrectFrameTime()
{
    while (true)
    {
        std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - timeAtStartOfFrame;
        if (elapsed.count() >= targetFrameTime)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }

    currentFPS = static_cast<int>(1 / (std::chrono::duration<double>(std::chrono::steady_clock::now() - timeAtStartOfFrame).count()));

    timeAtStartOfFrame = std::chrono::steady_clock::now();
}

void FPSManager::SetTargetFPS(int targetFPS)
{
    targetFrameTime = 1.0 / targetFPS;
}

int FPSManager::GetFPS()
{
    return currentFPS;
}