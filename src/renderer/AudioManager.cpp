// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#define MINIAUDIO_IMPLEMENTATION

#include "AudioManager.hpp"

#include "../shared/Log.hpp"

AudioManager::AudioManager()
{
    ma_result res = ma_engine_init(NULL, &miniaudio);

    if (res != MA_SUCCESS)
        Log::add('M', 200);

    Log::add('M', 000);
}

float AudioManager::attenuation(float distance) const
{
    float ref = 3.0f;
    float rolloff = 3.0f;
    return ref / (ref + rolloff * (distance - ref));
}

float AudioManager::volumeToGain(float volume) const
{
    if (volume <= 0.0f)
        return 0.0f;
    float dB = -60.0f + volume * 60.0f;
    return std::pow(10.0f, dB / 20.0f);
}

void AudioManager::tick(AudioData audioData)
{
    for (const VEAudioRequest &req : audioData.audioRequests)
    {
        bool foundAudio = false;
        for (VEAudio &audio : audios)
        {
            if (req.vehicleHandle == audio.vehicleHandle)
            {
                foundAudio = true;

                ma_sound_set_pitch(&audio.sound, 0.5f + req.pitch); // Temporary hardcoded base pitch offset

                float dx = req.position.x - audioData.playerPosition.x;
                float dy = req.position.y - audioData.playerPosition.y;
                float dz = req.position.z - audioData.playerPosition.z;

                float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

                float gain = volumeToGain(audioData.volume) * attenuation(distance);

                ma_sound_set_volume(&audio.sound, req.pitch == 0 ? 0 : gain); // If req.pitch == 0 => Engine RPM = 0

                float fx = cosf(audioData.playerYawRad);
                float fz = sinf(audioData.playerYawRad);

                float cross = fx * dz - fz * dx;

                float distanceXZ = std::sqrt(dx*dx + dz*dz);
                float pan = (distanceXZ > 1e-6f) ? (cross / distanceXZ) : 0.0f;
                pan = clamp(pan, -1.0f, 1.0f);

                ma_sound_set_pan(&audio.sound, pan);

                break;
            }
        }

        if (!foundAudio)
        {
            VEAudio newAudio;
            newAudio.vehicleHandle = req.vehicleHandle;
            audios.push_back(newAudio);

            ma_result res = ma_sound_init_from_file(&miniaudio, req.fileName.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &audios.back().sound);
            if (res != MA_SUCCESS)
            {
                if (!req.fileName.empty())
                    Log::add('M', 100);

                audios.pop_back();

                continue;
            }

            ma_sound_set_looping(&audios.back().sound, MA_TRUE);
            ma_sound_start(&audios.back().sound);

            ma_sound_set_volume(&audios.back().sound, 1.0f);
            ma_sound_set_pitch(&audios.back().sound, 1.0f);
        }
    }
}

AudioManager::~AudioManager()
{
    for (VEAudio &audio : audios)
    {
        ma_sound_uninit(&audio.sound);
    }
    ma_engine_uninit(&miniaudio);
}
