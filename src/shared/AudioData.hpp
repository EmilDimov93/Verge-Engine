// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

struct VEEngineAudioRequest
{
    VehicleHandle vehicleHandle;
    std::string fileName;
    float pitch;
    Position3 position;
};

struct VEEngineAudioFile
{
    std::string fileName;
    float rpm;
};

struct VELayeredEngineAudioRequest
{
    VehicleHandle vehicleHandle;
    std::vector<VEEngineAudioFile> audioFiles;
    float rpm;
    float maxRpm;
    Position3 position;
};

struct VEAudioRequest
{
    std::string fileName;
    float pitch;
    bool is3D;
    Position3 position;
};

struct AudioData
{
    Position3 playerPosition;
    float playerYawRad;
    const std::vector<VEEngineAudioRequest> &engineAudioRequests;
    const std::vector<VELayeredEngineAudioRequest> &layeredEngineAudioRequests;
    const std::vector<VEAudioRequest> &oneShotAudioRequests;

    AudioData(Position3 playerPosition,
              float playerYawRad,
              const std::vector<VEEngineAudioRequest> &engineAudioRequests,
              const std::vector<VELayeredEngineAudioRequest> &layeredEngineAudioRequests,
              const std::vector<VEAudioRequest> &oneShotAudioRequests)
        : playerPosition(playerPosition), playerYawRad(playerYawRad), engineAudioRequests(engineAudioRequests), layeredEngineAudioRequests(layeredEngineAudioRequests), oneShotAudioRequests(oneShotAudioRequests) {}
};