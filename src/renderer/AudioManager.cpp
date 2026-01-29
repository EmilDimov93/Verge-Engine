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

void AudioManager::tick(AudioData audioData)
{
    for (const VEAudioRequest &req : audioData.audioRequests)
    {
        bool foundAudio = false;
        for (VEAudio &audio2 : audios)
        {
            if (req.vehicleHandle == audio2.vehicleHandle)
            {
                foundAudio = true;

                ma_sound_set_volume(&audio2.sound, 0.01f);

                ma_sound_set_pan(&audio2.sound, 0.0f);

                ma_sound_set_pitch(&audio2.sound, 0.5f + req.pitch);
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
                if(!req.fileName.empty())
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
    for (VEAudio audio : audios)
    {
        ma_sound_uninit(&audio.sound);
    }
    ma_engine_uninit(&miniaudio);
}
