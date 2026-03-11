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

    if(fabsf(res) < inputEpsilon)
        res = 0.0f;

    return res;
}

VehicleInputState Renderer::getVIS()
{
    float dt = fps.getFrameTime();

    vis.throttle = keybinds.throttle.isAxis() ? keybinds.throttle.getValue()
                                              : smoothValue(keybinds.throttle.getValue(), vis.throttle, throttleSmoothing, dt);

    vis.brake = keybinds.brake.isAxis() ? keybinds.brake.getValue()
                                              : smoothValue(keybinds.brake.getValue(), vis.brake, brakeSmoothing, dt);

    vis.handbrake = keybinds.handbrake.isAxis() ? keybinds.handbrake.getValue()
                                              : smoothValue(keybinds.handbrake.getValue(), vis.handbrake, handbrakeSmoothing, dt);

    vis.clutch = keybinds.clutch.isAxis() ? keybinds.clutch.getValue()
                                              : smoothValue(keybinds.clutch.getValue(), vis.clutch, clutchSmoothing, dt);

    vis.steer = (keybinds.steerRight.isAxis() && keybinds.steerLeft.isAxis()) ? keybinds.steerLeft.getValue() - keybinds.steerRight.getValue()
                                              : smoothValue(keybinds.steerLeft.getValue() - keybinds.steerRight.getValue(), vis.steer, steerSmoothing, dt);

    if (!keybinds.shiftUp.isAxis() && !keybinds.shiftDown.isAxis())
    {
        if (keybinds.shiftUp.isPressed())
        {
            vis.shiftUp = true;
        }
        if (keybinds.shiftDown.isPressed())
        {
            vis.shiftDown = true;
        }
    }

    vis.starter = keybinds.startEngine.getValue() > 0.0f;

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
