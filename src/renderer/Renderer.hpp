// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "WindowManager.hpp"
#include "VulkanManager.hpp"
#include "AudioManager.hpp"
#include "Input.hpp"
#include "FPSManager.hpp"

#include "../shared/Log.hpp"

#define VE_KEYBIND_COUNT 2

struct VERendererCreateInfo
{
    std::string projectName = "Verge Engine Program";
    Size2 windowSize = {};
    VELogOutputMode logOutputMode = VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE;
    uint16_t targetFps = VE_DEFAULT_FPS;
};

struct VEVehicleKeybinds
{
    VEKeybind throttle[VE_KEYBIND_COUNT];
    VEKeybind brake[VE_KEYBIND_COUNT];
    VEKeybind handbrake[VE_KEYBIND_COUNT];
    VEKeybind clutch[VE_KEYBIND_COUNT];

    VEKeybind steerLeft[VE_KEYBIND_COUNT];
    VEKeybind steerRight[VE_KEYBIND_COUNT];

    VEKeybind shiftUp[VE_KEYBIND_COUNT];
    VEKeybind shiftDown[VE_KEYBIND_COUNT];

    VEKeybind startEngine[VE_KEYBIND_COUNT];

    VEKeybind moveCameraLeft[VE_KEYBIND_COUNT];
    VEKeybind moveCameraRight[VE_KEYBIND_COUNT];
    VEKeybind moveCameraUp[VE_KEYBIND_COUNT];
    VEKeybind moveCameraDown[VE_KEYBIND_COUNT];
};

class Renderer
{
public:
    Renderer(const VERendererCreateInfo &info = {});

    bool isOpen();

    void tick(const DrawData &drawData, const AudioData &audioData);

    VehicleInputState getVIS();
    void setVehicleKeybinds(const VEVehicleKeybinds &keybinds);

    ~Renderer();

    ve_time_t getFrameTime() const;
    uint32_t getFps() const;
    void setTargetFps(uint16_t target);

    float getVolume() const;
    void setVolume(float volume);

    void setThrottleInputSmoothing(float smoothing) { throttleSmoothing = clamp01(smoothing); }
    void setBrakeInputSmoothing(float smoothing) { brakeSmoothing = clamp01(smoothing); }
    void setHandbrakeInputSmoothing(float smoothing) { handbrakeSmoothing = clamp01(smoothing); }
    void setClutchInputSmoothing(float smoothing) { clutchSmoothing = clamp01(smoothing); }
    void setSteerInputSmoothing(float smoothing) { steerSmoothing = clamp01(smoothing); }
    void setCameraMovementInputSmoothing(float smoothing) { cameraMovementSmoothing = clamp01(smoothing); }

    // void setAspectRatio(float aspectRatio);
    void setFOV(float fov);
    void setzNear(float zNear);
    void setZFar(float zFar);

private:
    // Window & Rendering
    WindowManager window;
    VulkanManager vulkan;
    FpsManager fps;

    float aspectRatio = 1.0f;
    float fov = 60.0f;
    float zNear = 0.01f;
    float zFar = 1000.0f;

    glm::mat4 getProjectionMat() const;

    // Audio
    AudioManager audio;
    float volume = 1.0f;

    // Input
    VEVehicleKeybinds keybinds;
    VehicleInputState vis;

    // Input smoothing for non-axis keybinds
    // 0 -> no smoothing (instant response)
    // 1 -> maximum smoothing
    float throttleSmoothing = 0.5f;
    float brakeSmoothing = 0.5f;
    float handbrakeSmoothing = 0.0f;
    float clutchSmoothing = 0.5f;
    float steerSmoothing = 0.5f;
    float cameraMovementSmoothing = 0.0f;
};