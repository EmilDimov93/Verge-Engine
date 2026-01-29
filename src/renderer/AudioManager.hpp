// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../ext/miniaudio/miniaudio.h"

#include "../shared/definitions.hpp"

struct VEAudio
{
    VehicleHandle vehicleHandle;
    ma_sound sound;
};

class AudioManager
{
public:
    AudioManager();

    void tick(AudioData audioData);

    ~AudioManager();

private:
    std::vector<VEAudio> audios;

    ma_engine miniaudio;
};