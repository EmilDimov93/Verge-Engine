#pragma once

#include <chrono>

class FPSManager{
    private:
        std::chrono::steady_clock::time_point timeAtStartOfFrame;
        double targetFrameTime;
    public:
        void CorrectFrameTime();
        void CaptureFrameStartTime();
        void SetTargetFPS(int targetFps);
        int GetFPS();
};