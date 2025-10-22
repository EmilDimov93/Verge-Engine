#include "FPSManager.h"

void FPSManager::CorrectFrameTime()
{
    while (true)
    {
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - timeAtStartOfFrame;
        if (elapsed.count() >= targetFrameTime){
            break;
        }
    }
}

void FPSManager::CaptureFrameStartTime()
{
    timeAtStartOfFrame = std::chrono::high_resolution_clock::now();
}

void FPSManager::SetTargetFPS(int targetFPS){
    targetFrameTime = 1.0 / targetFPS;
}

int FPSManager::GetFPS(){
    return static_cast<int>(1 / (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - timeAtStartOfFrame).count()));
}