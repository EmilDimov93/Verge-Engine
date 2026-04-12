// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

#include <chrono>

namespace VE
{

    constexpr uint16_t DEFAULT_FPS = 140;

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
        milliseconds_t targetFrameTime = 0;

        milliseconds_t lastFrameTime = 0;
        uint16_t currentFps = 0;
    };

}