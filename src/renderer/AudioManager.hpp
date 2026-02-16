// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

#include "../../ext/miniaudio/miniaudio.h"

#include <deque>

struct VEEngineAudio
{
    VehicleHandle vehicleHandle;
    ma_sound sound;
};

struct VEAudio
{
    ma_sound sound;
    bool is3D;
    Position3 position;
};

class AudioManager
{
public:
    AudioManager();

    void tick(AudioData audioData);

    ~AudioManager();

private:
    std::deque<VEEngineAudio> engineAudios;
    std::deque<VEAudio> oneShotAudios;

    ma_engine miniaudio;

    float volumeToGain(float volume) const;
    float attenuation(float distance) const;
};