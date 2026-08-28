// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

#include "../shared/log.hpp"

namespace VE
{

    Client::Client(const ClientCreateInfo &info) : window(info.windowSize, info.projectName), renderer(window.ptr(), window.getSize()), keybinds(0)
    {
        Input::init(window.ptr());
        Log::init(info.logOutputMode);
        fps.setTarget(info.targetFps);
    }

    bool Client::isOpen()
    {
        return window.isOpen();
    }

    void Client::tick(const SceneDrawData &sceneDrawData, const AudioData &audioData)
    {
        Input::refresh();

        renderer.drawFrame(sceneDrawData, ui.getWidgetData(), getProjectionMat(), bindedEffects);

        audio.tick(audioData, volume);

        glm::vec2 mousePos = Input::getMousePos();
        glm::uvec2 windowSize = window.getSize();
        ui.tick(Input::isPressed(MOUSE_BTN_LEFT), mousePos, window.getSize());

        fps.sync();
    }

    float smoothValue(float newValue, float oldValue, float smoothingFactor, float dt)
    {
        if (smoothingFactor <= 0.f)
            return newValue;

        const bool differentSigns = (newValue >= 0.f && oldValue <= 0.f) || (newValue <= 0.f && oldValue >= 0.f);

        if (differentSigns || fabsf(newValue) < fabsf(oldValue))
            smoothingFactor = 1e-5f;

        const float interpolationSpeed = (1.f - smoothingFactor) * 10.f;

        const float bigger = newValue > oldValue ? newValue : oldValue;
        const float smaller = newValue < oldValue ? newValue : oldValue;

        const float inputEpsilon = 1e-5f;

        float res = clamp(oldValue + (newValue - oldValue) * interpolationSpeed * dt, smaller, bigger);

        if (fabsf(res) < inputEpsilon)
            res = 0.f;

        return res;
    }

    float getMaxAbsKeybindValue(const std::vector<Keybind>& keybindArray, bool &isAxis)
    {
        float maxValue = 0.f;
        isAxis = false;

        for (const Keybind& keybind : keybindArray)
        {
            float currValue = keybind.getValue();
            if (fabsf(currValue) > fabsf(maxValue))
            {
                maxValue = currValue;
                isAxis = keybind.isAxis();
            }
        }

        return maxValue;
    }

    VehicleInputState Client::getVIS()
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

            for (Keybind &k : keybinds.shiftUp)
            {
                if (!k.isAxis() && k.isPressed())
                {
                    vis.shiftUp = true;
                }
            }
        }

        {
            vis.shiftDown = false;
            for (Keybind &k : keybinds.shiftDown)
            {
                if (!k.isAxis() && k.isPressed())
                {
                    vis.shiftDown = true;
                }
            }
        }

        {
            bool isStartEngineAxis = false;
            float startEngine = getMaxAbsKeybindValue(keybinds.startEngine, isStartEngineAxis);

            vis.starter = startEngine > 0.f;
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

    void Client::setVehicleKeybinds(const VehicleKeybinds &keybinds)
    {
        this->keybinds = keybinds;
    }

    Client::~Client()
    {
        Log::end();
    }

    seconds_t Client::getFrameTime() const
    {
        return fps.getFrameTime();
    }

    uint32_t Client::getFps() const
    {
        return fps.getFps();
    }

    void Client::setTargetFps(uint16_t target)
    {
        fps.setTarget(target);
    }

    void Client::setVolume(float volume)
    {
        if (volume >= 0 && volume <= 1.f)
            this->volume = volume;
    }

    void Client::setFOV(float fov)
    {
        if (fov > 0.f && fov < 180.f)
            this->fov = fov;
        else
            Log::add('C', 200);
    }

    void Client::setzNear(float zNear)
    {
        if (zNear > 0.f)
            this->zNear = zNear;
        else
            Log::add('C', 201);
    }

    void Client::setZFar(float zFar)
    {
        if (zFar > zNear)
            this->zFar = zFar;
        else
            Log::add('C', 202);
    }

    glm::mat4 Client::getProjectionMat() const
    {
        glm::mat4 projectionMat = glm::perspective(glm::radians(fov), window.getAspectRatio(), zNear, zFar);
        projectionMat[1][1] *= -1;
        return projectionMat;
    }

    float Client::getVolume() const
    {
        return volume;
    }

}