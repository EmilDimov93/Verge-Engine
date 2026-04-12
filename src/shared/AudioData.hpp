// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

namespace VE
{

    struct EngineAudioRequest
    {
        VehicleHandle vehicleHandle;
        std::string fileName;
        float pitch;
        Position3 position;
    };

    struct EngineAudioFileRequest
    {
        std::string fileName;
        float rpm;
    };

    struct LayeredEngineAudioRequest
    {
        VehicleHandle vehicleHandle;
        std::vector<EngineAudioFileRequest> audioFiles;
        float rpm;
        float maxRpm;
        Position3 position;
    };

    struct AudioRequest
    {
        std::string fileName;
        float pitch;
        bool is3D;
        Position3 position;
    };

    struct AudioData
    {
        const Position3 playerPosition;
        const float playerYawRad;
        const std::vector<EngineAudioRequest> &engineAudioRequests;
        const std::vector<LayeredEngineAudioRequest> &layeredEngineAudioRequests;
        const std::vector<AudioRequest> &oneShotAudioRequests;
        const bool vehicleRemovedThisFrame;

        AudioData(const Position3 playerPosition,
                  const float playerYawRad,
                  const std::vector<EngineAudioRequest> &engineAudioRequests,
                  const std::vector<LayeredEngineAudioRequest> &layeredEngineAudioRequests,
                  const std::vector<AudioRequest> &oneShotAudioRequests,
                  const bool vehicleRemovedThisFrame)
            : playerPosition(playerPosition), playerYawRad(playerYawRad), engineAudioRequests(engineAudioRequests), layeredEngineAudioRequests(layeredEngineAudioRequests), oneShotAudioRequests(oneShotAudioRequests), vehicleRemovedThisFrame(vehicleRemovedThisFrame) {}
    };

}