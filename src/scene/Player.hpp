// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Controller.hpp"

#include "Camera.hpp"
#include "../shared/Input.hpp"

struct PlayerKeybinds
{
    VEKeybind throttle;
    VEKeybind brake;
    VEKeybind handbrake;
    VEKeybind clutch;

    VEKeybind steerLeft;
    VEKeybind steerRight;

    VEKeybind shiftUp;
    VEKeybind shiftDown;
};

class Player : public Controller
{
public:
    Player(PlayerHandle handle, VehicleHandle vehicleHandle, const PlayerKeybinds &keybinds, const VE_STRUCT_CAMERA_CREATE_INFO &cameraInfo)
        : handle(handle), camera(cameraInfo)
    {
        this->vehicleHandle = vehicleHandle;
        this->keybinds = keybinds;
    }

    VehicleInputState getVehicleInputState() const override
    {
        VehicleInputState vis{};

        if (keybinds.throttle.isAxis())
        {
            vis.throttle = keybinds.throttle.getAxis();
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
            vis.brake = keybinds.brake.getAxis();
        }
        else
        {
            if (keybinds.brake.isDown())
                vis.brake = 1.0f;
            else
                vis.brake = 0.0f;
        }

        if(keybinds.handbrake.isAxis())
        {
            vis.handbrake = keybinds.handbrake.getAxis() > 0;
        }
        else
        {
            vis.handbrake = keybinds.handbrake.isDown();
        }

        vis.clutch = keybinds.clutch.isAxis() ? fabs(keybinds.clutch.getAxis() - 1.0f) : (keybinds.clutch.isDown() ? 0.0f : 1.0f);

        float rightSteerValue = keybinds.steerRight.isAxis() ? keybinds.steerRight.getAxis() : (keybinds.steerRight.isDown() ? 1.0f : 0.0f);
        float leftSteerValue = keybinds.steerLeft.isAxis() ? keybinds.steerLeft.getAxis() : (keybinds.steerLeft .isDown()  ? 1.0f : 0.0f);

        vis.steer = rightSteerValue - leftSteerValue;

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

        return vis;
    }

    void setVehicleHandle(VehicleHandle vehicleHandle)
    {
        this->vehicleHandle = vehicleHandle;
    }

    VehicleHandle getVehicleHandle() const override
    {
        return vehicleHandle;
    }

    PlayerHandle getHandle() const
    {
        return handle;
    }

    void updateCamera(ve_time_t dt, Transform vehicleTransform, glm::vec3 vehicleVelocityVector)
    {
        if (!isCameraFollowingVehicle)
            return;

        float currCameraHeight = cameraFollowHeight;
        
        // Move camera when ground is obstructing view
        /*if(ground.sampleHeight(camera.getPosition().x, camera.getPosition().z) >= currCameraHeight){
            currCameraHeight += ground.sampleHeight(camera.getPosition().x, camera.getPosition().z);
        }*/

        float targetYaw = atan2(vehicleVelocityVector.x, vehicleVelocityVector.z) - PI;

        Position3 vehiclePos = vehicleTransform.position;

        camPos = {camera.getPosition().x, camera.getPosition().y, camera.getPosition().z};
        glm::vec3 targetCamPos = {vehiclePos.x + sin(cameraYaw) * cameraFollowDistance, vehiclePos.y + currCameraHeight, vehiclePos.z + cos(cameraYaw) * cameraFollowDistance};
        camPos = glm::mix(camPos, targetCamPos, 1.0f - std::exp(-float(dt) * 10.0f));
        camera.move({camPos.x, camPos.y, camPos.z});

        cameraYaw += (targetYaw - cameraYaw) * cameraFollowDelay;

        glm::vec3 dir = glm::normalize(glm::vec3(vehiclePos.x, vehiclePos.y, vehiclePos.z) - camPos);
        camera.rotate({asin(dir.y), atan2(dir.z, dir.x), 0});
    }

    void setCameraFollowDistance(float distance)
    {
        cameraFollowDistance = distance;
    }

    void setCameraFollowHeight(float height)
    {
        cameraFollowHeight = height;
    }

    void setCameraFollowDelay(float delay)
    {
        cameraFollowDelay = delay;
    }

    void setCameraFollowVehicle(bool shouldFollow)
    {
        isCameraFollowingVehicle = shouldFollow;
    }

    glm::mat4 getCameraProjectionMat() const
    {
        return camera.getProjectionMat();
    }

    glm::mat4 getCameraViewMat() const
    {
        return camera.getViewMat();
    }

    Position3 getCameraPosition() const
    {
        return camera.getPosition();
    }

    float getCameraYaw() const
    {
        return camera.getRotation().yaw;
    }

    float setVolume(float volume)
    {
        if(volume >= 0 && volume <= 1.0f)
            this->volume = volume;
    }

    float getVolume() const
    {
        return volume;
    }

private:
    const PlayerHandle handle;

    VehicleHandle vehicleHandle;

    Camera camera;

    PlayerKeybinds keybinds;

    float cameraYaw = -PI;
    glm::vec3 camPos;

    bool isCameraFollowingVehicle = true;
    float cameraFollowDistance = 10.0f;
    float cameraFollowHeight = 3.0f;
    float cameraFollowDelay = 0.01f;

    float volume = 1.0f;
};