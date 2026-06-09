// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer/Renderer.hpp"
#include "WindowManager.hpp"
#include "AudioManager.hpp"
#include "Input.hpp"
#include "FPSManager.hpp"

#include "UI.hpp"

namespace VE
{
    struct ClientCreateInfo
    {
        std::string projectName = "Verge Engine Program";
        Size2 windowSize = {};
        LogOutputMode logOutputMode = LOG_OUTPUT_MODE_FILE_AND_CONSOLE;
        uint16_t targetFps = DEFAULT_FPS;
    };

    class VehicleKeybinds
    {
    public:
        explicit VehicleKeybinds(uint32_t count)
            : throttle(count), brake(count), handbrake(count), clutch(count), steerLeft(count), steerRight(count), shiftUp(count), shiftDown(count), startEngine(count), moveCameraLeft(count), moveCameraRight(count), moveCameraUp(count), moveCameraDown(count)
        {
        }

        std::vector<Keybind> throttle;
        std::vector<Keybind> brake;
        std::vector<Keybind> handbrake;
        std::vector<Keybind> clutch;

        std::vector<Keybind> steerLeft;
        std::vector<Keybind> steerRight;

        std::vector<Keybind> shiftUp;
        std::vector<Keybind> shiftDown;

        std::vector<Keybind> startEngine;

        std::vector<Keybind> moveCameraLeft;
        std::vector<Keybind> moveCameraRight;
        std::vector<Keybind> moveCameraUp;
        std::vector<Keybind> moveCameraDown;
    };

    class Client
    {
    public:
        Client(const ClientCreateInfo &info = {});

        [[nodiscard]] bool isOpen();

        void tick(const SceneDrawData &sceneDrawData, const AudioData &audioData);

        void bindPostEffects(PostEffects effects) { bindedEffects = effects; };

        [[nodiscard]] VehicleInputState getVIS();
        void setVehicleKeybinds(const VehicleKeybinds &keybinds);

        ~Client();

        [[nodiscard]] milliseconds_t getFrameTime() const;
        [[nodiscard]] uint32_t getFps() const;
        void setTargetFps(uint16_t target);

        [[nodiscard]] float getVolume() const;
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

        UI ui;

    private:
        // Window & Rendering
        WindowManager window;
        Renderer renderer;
        FpsManager fps;

        PostEffects bindedEffects;

        float aspectRatio = 1.0f;
        float fov = 60.0f;
        float zNear = 0.01f;
        float zFar = 1000.0f;

        [[nodiscard]] glm::mat4 getProjectionMat() const;

        // Audio
        AudioManager audio;
        float volume = 1.0f;

        // Input
        VehicleKeybinds keybinds;
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

}