// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Client.hpp"

#include "../shared/Log.hpp"

namespace VE
{

    Client::Client(const ClientCreateInfo &info) : window(info.windowSize, info.projectName), renderer(window.getReference(), window.getSize())
    {
        Input::init(window.getReference());
        Log::init(info.logOutputMode);
        fps.setTarget(info.targetFps);
        aspectRatio = window.getAspectRatio();

        glfwSetWindowUserPointer(window.getReference(), this);

        glfwSetFramebufferSizeCallback(window.getReference(), [](GLFWwindow *window, int width, int height)
                                       {
                                        Client *client = reinterpret_cast<Client*>(glfwGetWindowUserPointer(window));
    
                                        if (height == 0) return; 

                                        client->aspectRatio = static_cast<float>(width) / static_cast<float>(height);
                                        
                                        client->renderer.markFramebufferResized(); });
    }

    bool Client::isOpen()
    {
        return window.isOpen();
    }

    void Client::tick(const SceneDrawData &sceneDrawData, const AudioData &audioData)
    {
        Input::refresh();

        renderer.drawFrame(sceneDrawData, ui.getWidgetData(), getProjectionMat());

        audio.tick(audioData, volume);

        Position2 mousePos = Input::getMousePos();
        Size2 windowSize = window.getSize();
        ui.tick(Input::isPressed(MOUSE_BTN_LEFT), Position2(2 * mousePos.x / windowSize.w - 1, 2 * mousePos.y / windowSize.h - 1));

        fps.sync();
    }

    float smoothValue(float newValue, float oldValue, float smoothingFactor, float dt)
    {
        if (smoothingFactor <= 0.0f)
            return newValue;

        const bool differentSigns = (newValue >= 0.0f && oldValue <= 0.0f) || (newValue <= 0.0f && oldValue >= 0.0f);

        if (differentSigns || fabsf(newValue) < fabsf(oldValue))
            smoothingFactor = 1e-5f;

        const float interpolationSpeed = (1.0f - smoothingFactor) * 10.0f;

        const float bigger = newValue > oldValue ? newValue : oldValue;
        const float smaller = newValue < oldValue ? newValue : oldValue;

        const float inputEpsilon = 1e-5f;

        float res = clamp(oldValue + (newValue - oldValue) * interpolationSpeed * dt, smaller, bigger);

        if (fabsf(res) < inputEpsilon)
            res = 0.0f;

        return res;
    }

    float getMaxAbsKeybindValue(const Keybind keybindArray[KEYBIND_COUNT], bool &isAxis)
    {
        float maxValue = 0.0f;
        isAxis = false;

        for (int keybindIndex = 0; keybindIndex < KEYBIND_COUNT; keybindIndex++)
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

    void Client::setVehicleKeybinds(const VehicleKeybinds &keybinds)
    {
        this->keybinds = keybinds;
    }

    Client::~Client()
    {
        Log::end();
    }

    milliseconds_t Client::getFrameTime() const
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
        if (volume >= 0 && volume <= 1.0f)
            this->volume = volume;
    }

    void Client::setFOV(float fov)
    {
        if (fov > 0.0f && fov < 180.0f)
            this->fov = fov;
        else
            Log::add('R', 200);
    }

    void Client::setzNear(float zNear)
    {
        if (zNear > 0.0f)
            this->zNear = zNear;
        else
            Log::add('R', 201);
    }

    void Client::setZFar(float zFar)
    {
        if (zFar > zNear)
            this->zFar = zFar;
        else
            Log::add('R', 202);
    }

    glm::mat4 Client::getProjectionMat() const
    {
        glm::mat4 projectionMat = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);
        projectionMat[1][1] *= -1;
        return projectionMat;
    }

    float Client::getVolume() const
    {
        return volume;
    }

}