// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WindowManager.hpp"
#include "VulkanManager.hpp"
#include "AudioManager.hpp"
#include "Input.hpp"
#include "FPSManager.hpp"

#include "../shared/Log.hpp"

struct VE_STRUCT_RENDERER_CREATE_INFO
{
    std::string projectName = "Verge Engine Program";
    Size2 windowSize = {};
    LogOutputMode logOutputMode = VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE;
    uint16_t targetFps = VE_DEFAULT_FPS;
};

struct VehicleKeybinds
{
    VEKeybind throttle;
    VEKeybind brake;
    VEKeybind handbrake;
    VEKeybind clutch;

    VEKeybind steerLeft;
    VEKeybind steerRight;

    VEKeybind shiftUp;
    VEKeybind shiftDown;

    VEKeybind startEngine;

    VEKeybind moveCameraLeft;
    VEKeybind moveCameraRight;
    VEKeybind moveCameraUp;
    VEKeybind moveCameraDown;
};

class Renderer
{
public:
    Renderer(const VE_STRUCT_RENDERER_CREATE_INFO &info = {});

    bool tick(DrawData drawData, AudioData audioData);

    VehicleInputState getVIS();
    void setVehicleKeybinds(const VehicleKeybinds &keybinds);

    ~Renderer();

    ve_time_t getFrameTime() const;
    uint32_t getFps() const;
    void setTargetFps(uint16_t target);

    float getAspectRatio() const;

    float getVolume() const;
    void setVolume(float volume);

    // Input smoothing for non-axis keybinds
    // 0 -> no smoothing (instant response)
    // 1 -> maximum smoothing
    float throttleSmoothing = 0.5f;
    float brakeSmoothing = 0.5f;
    float handbrakeSmoothing = 0.0f;
    float clutchSmoothing = 0.5f;
    float steerSmoothing = 0.5f;

private:
    WindowManager window;
    VulkanManager vulkan;
    AudioManager audio;
    FpsManager fps;

    float volume = 1.0f;

    VehicleKeybinds keybinds;
    VehicleInputState vis;
};