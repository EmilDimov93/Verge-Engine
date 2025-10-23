#pragma once

#include <chrono>

class FPSManager
{
private:
    std::chrono::steady_clock::time_point timeAtStartOfFrame;
    double targetFrameTime;
    int currentFPS;

public:
    void CorrectFrameTime();
    void SetTargetFPS(int targetFps);
    int GetFPS();
};