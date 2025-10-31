#pragma once

#include <chrono>

class FpsManager
{
private:
    std::chrono::steady_clock::time_point timeAtStartOfFrame;
    double targetFrameTime;
    int currentFps;

public:
    void syncFrameTime();
    void setTargetFps(int targetFps);
    int getFps();
};