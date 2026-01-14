// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Controller.hpp"

#include "Camera.hpp"
#include "../shared/Input.hpp"

struct PlayerVehicleKeybinds
{
    VEKeybind throttle;
    VEKeybind brake;

    VEKeybind steerAxis;
    VEKeybind steerLeft;
    VEKeybind steerRight;

    VEKeybind shiftUp;
    VEKeybind shiftDown;
};

class Player : public Controller
{
public:
    Player(const PlayerVehicleKeybinds& keybinds, const VE_STRUCT_CAMERA_CREATE_INFO& cameraInfo) : camera(cameraInfo)
    {
        this->keybinds = keybinds;
    }

    const VehicleInputState getVehicleInputState() override
    {
        VehicleInputState vis{};

        if (keybinds.throttle.isAxis())
        {
            vis.throttle = keybinds.throttle.getAxisNormalized();
        }
        else
        {
            if (keybinds.throttle.isDown())
                vis.throttle = 1.0f;
            else
                vis.throttle = 0.0f;
        }

        if (keybinds.brake.isAxis())
        {
            vis.brake = keybinds.brake.getAxisNormalized();
        }
        else
        {
            if (keybinds.brake.isDown())
                vis.brake = 1.0f;
            else
                vis.brake = 0.0f;
        }

        vis.steer = -keybinds.steerAxis.getAxis();

        if(keybinds.steerLeft.isDown() && keybinds.steerRight.isUp()){
            vis.steer = 1.0f;
        }

        if(keybinds.steerRight.isDown() && keybinds.steerLeft.isUp()){
            vis.steer = -1.0f;
        }

        if (!keybinds.shiftUp.isAxis() && !keybinds.shiftDown.isAxis())
        {
            if(keybinds.shiftUp.isPressed()){
                vis.shiftUp = true;
            }
            if(keybinds.shiftDown.isPressed()){
                vis.shiftDown = true;
            }
        }

        return vis;
    }

    void setVehicleIndex(uint64_t vehicleIndex){
        this->vehicleIndex = vehicleIndex;
    }

    const uint64_t getVehicleIndex() override{
        return vehicleIndex;
    }

    // Temporarily public
    Camera camera;

private:
    

    uint64_t vehicleIndex;

    PlayerVehicleKeybinds keybinds;
};