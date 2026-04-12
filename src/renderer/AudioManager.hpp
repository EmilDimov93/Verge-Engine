// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/AudioData.hpp"
#include "../shared/definitions.hpp"

#include "../../ext/miniaudio/miniaudio.h"

#include <deque>

namespace VE
{

struct EngineAudio
{
    VehicleHandle vehicleHandle;
    ma_sound sound;
};

struct EngineAudioFile
{
    ma_sound sound;
    float rpm;
};

struct LayeredEngineAudio
{
    VehicleHandle vehicleHandle;
    std::vector<EngineAudioFile> audioFiles;
};

struct Audio
{
    ma_sound sound;
    bool is3D;
    Position3 position;
};

class AudioManager
{
public:
    AudioManager();

    void tick(const AudioData& audioData, float volume);

    ~AudioManager();

private:
    std::deque<EngineAudio> engineAudios;
    std::deque<LayeredEngineAudio> layeredEngineAudios;
    std::deque<Audio> oneShotAudios;

    ma_engine miniaudio;

    void removeOrphanedAudio(const AudioData& audioData);

    float volumeToGain(float volume) const;
    float attenuation(float distance) const;
};

}