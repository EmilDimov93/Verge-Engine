// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Renderer.hpp"

#include "../shared/Log.hpp"

Renderer::Renderer(const VE_STRUCT_RENDERER_CREATE_INFO &info) : window(info.windowSize, info.projectName), vulkan(window.getReference(), window.getSize())
{
    Input::init(window.getReference());
    Log::init(info.logOutputMode);
    fps.setTarget(info.targetFps);
}

bool Renderer::tick(DrawData drawData, AudioData audioData)
{
    fps.sync();

    Input::refresh();

    vulkan.drawFrame(drawData);

    audio.tick(audioData, volume);

    return window.isOpen();
}

float smoothValue(float newValue, float oldValue, float smoothingFactor, float dt)
{
    if (smoothingFactor <= 0.0f)
        return newValue;

    float interpolationSpeed = (1.0f - smoothingFactor) * 10.0f;

    float bigger = newValue > oldValue ? newValue : oldValue;
    float smaller = newValue < oldValue ? newValue : oldValue;

    const float inputEpsilon = 1e-5f;

    float res = clamp(oldValue + (newValue - oldValue) * interpolationSpeed * dt, smaller, bigger);

    if (fabsf(res) < inputEpsilon)
        res = 0.0f;

    return res;
}

float getMaxAbsKeybindValue(const VEKeybind keybindArray[VE_KEYBIND_COUNT], bool &isAxis)
{
    float maxValue = 0.0f;
    isAxis = false;

    for (int keybindIndex = 0; keybindIndex < VE_KEYBIND_COUNT; keybindIndex++)
    {
        float currValue = keybindArray[keybindIndex].getValue();
        if (fabsf(currValue) > fabsf(maxValue))
        {
            maxValue = currValue;
            isAxis = keybindArray[keybindIndex].isAxis();
        }
    }

    return maxValue;
}

VehicleInputState Renderer::getVIS()
{
    float dt = fps.getFrameTime();

    {
        bool isThrottleAxis = false;
        float throttle = getMaxAbsKeybindValue(keybinds.throttle, isThrottleAxis);

        vis.throttle = isThrottleAxis ? throttle : smoothValue(throttle, vis.throttle, throttleSmoothing, dt);
    }

    {
        bool isBrakeAxis = false;
        float brake = getMaxAbsKeybindValue(keybinds.brake, isBrakeAxis);

        vis.brake = isBrakeAxis ? brake : smoothValue(brake, vis.brake, brakeSmoothing, dt);
    }

    {
        bool isHandbrakeAxis = false;
        float handbrake = getMaxAbsKeybindValue(keybinds.handbrake, isHandbrakeAxis);

        vis.handbrake = isHandbrakeAxis ? handbrake : smoothValue(handbrake, vis.handbrake, handbrakeSmoothing, dt);
    }

    {
        bool isClutchAxis = false;
        float clutch = getMaxAbsKeybindValue(keybinds.clutch, isClutchAxis);

        vis.clutch = isClutchAxis ? clutch : smoothValue(clutch, vis.clutch, clutchSmoothing, dt);
    }

    {
        bool isSteerLeftAxis = false;
        float steerLeft = getMaxAbsKeybindValue(keybinds.steerLeft, isSteerLeftAxis);

        bool isSteerRightAxis = false;
        float steerRight = getMaxAbsKeybindValue(keybinds.steerRight, isSteerRightAxis);

        vis.steer = (isSteerLeftAxis || isSteerRightAxis) ? steerLeft - steerRight : smoothValue(steerLeft - steerRight, vis.steer, steerSmoothing, dt);
    }

    {
        vis.shiftUp = false;

        for(VEKeybind &k : keybinds.shiftUp)
        {
            if(!k.isAxis() && k.isPressed()){
                vis.shiftUp = true;
            }
        }
    }

    {
        vis.shiftDown = false;
        for(VEKeybind &k : keybinds.shiftDown)
        {
            if(!k.isAxis() && k.isPressed()){
                vis.shiftDown = true;
            }
        }
    }

    {
        bool isStartEngineAxis = false;
        float startEngine = getMaxAbsKeybindValue(keybinds.startEngine, isStartEngineAxis);

        vis.starter = startEngine > 0.0f;
    }

    {
        bool isMoveCameraLeftAxis = false;
        float moveCameraLeft = getMaxAbsKeybindValue(keybinds.moveCameraLeft, isMoveCameraLeftAxis);

        vis.moveCameraLeft = isMoveCameraLeftAxis ? moveCameraLeft : smoothValue(moveCameraLeft, vis.moveCameraLeft, cameraMovementSmoothing, dt);
    }

    {
        bool isMoveCameraRightAxis = false;
        float moveCameraRight = getMaxAbsKeybindValue(keybinds.moveCameraRight, isMoveCameraRightAxis);

        vis.moveCameraRight = isMoveCameraRightAxis ? moveCameraRight : smoothValue(moveCameraRight, vis.moveCameraRight, cameraMovementSmoothing, dt);
    }

    {
        bool isMoveCameraUpAxis = false;
        float moveCameraUp = getMaxAbsKeybindValue(keybinds.moveCameraUp, isMoveCameraUpAxis);

        vis.moveCameraUp = isMoveCameraUpAxis ? moveCameraUp : smoothValue(moveCameraUp, vis.moveCameraUp, cameraMovementSmoothing, dt);
    }

    {
        bool isMoveCameraDownAxis = false;
        float moveCameraDown = getMaxAbsKeybindValue(keybinds.moveCameraDown, isMoveCameraDownAxis);

        vis.moveCameraDown = isMoveCameraDownAxis ? moveCameraDown : smoothValue(moveCameraDown, vis.moveCameraDown, cameraMovementSmoothing, dt);
    }

    return vis;
}

void Renderer::setVehicleKeybinds(const VehicleKeybinds &keybinds)
{
    this->keybinds = keybinds;
}

Renderer::~Renderer()
{
    Log::end();
}

ve_time_t Renderer::getFrameTime() const
{
    return fps.getFrameTime();
}

uint32_t Renderer::getFps() const
{
    return fps.getFps();
}

void Renderer::setTargetFps(uint16_t target)
{
    fps.setTarget(target);
}

float Renderer::getAspectRatio() const
{
    return window.getAspectRatio();
}

void Renderer::setVolume(float volume)
{
    if (volume >= 0 && volume <= 1.0f)
        this->volume = volume;
}

float Renderer::getVolume() const
{
    return volume;
}
