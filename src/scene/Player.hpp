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

    VEKeybind steerAxis;
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

        if (keybinds.steerLeft.isDown() && keybinds.steerRight.isUp())
        {
            vis.steer = 1.0f;
        }

        if (keybinds.steerRight.isDown() && keybinds.steerLeft.isUp())
        {
            vis.steer = -1.0f;
        }

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
        camera.rotate({glm::degrees(asin(dir.y)), glm::degrees(atan2(dir.z, dir.x)), 0});
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
};