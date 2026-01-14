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
    Player(VehicleId vehicleId, const PlayerKeybinds &keybinds, const VE_STRUCT_CAMERA_CREATE_INFO &cameraInfo) : camera(cameraInfo)
    {
        this->vehicleId = vehicleId;
        this->keybinds = keybinds;
    }

    VehicleInputState getVehicleInputState() override
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

    void setVehicleId(VehicleId vehicleId)
    {
        this->vehicleId = vehicleId;
    }

    VehicleId getVehicleId() const override
    {
        return vehicleId;
    }

    PlayerId getId() const
    {
        return id;
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

        float vehicleYaw = atan2(vehicleVelocityVector.x, vehicleVelocityVector.z);

        static float cameraYaw = vehicleYaw - PI;

        float targetYaw = vehicleYaw - PI;

        cameraYaw += (targetYaw - cameraYaw) * cameraFollowDelay;

        Position3 vehiclePos = vehicleTransform.position;

        static glm::vec3 prevCamPos = {camera.getPosition().x, camera.getPosition().y, camera.getPosition().z};
        glm::vec3 targetCamPos = {vehiclePos.x + sin(cameraYaw) * cameraFollowDistance, vehiclePos.y + currCameraHeight, vehiclePos.z + cos(cameraYaw) * cameraFollowDistance};
        glm::vec3 newCamPos = glm::mix(prevCamPos, targetCamPos, 1.0f - std::exp(-float(dt) * 10.0f));
        camera.move({newCamPos.x, newCamPos.y, newCamPos.z});
        prevCamPos = {camera.getPosition().x, camera.getPosition().y, camera.getPosition().z};

        glm::vec3 dir = glm::normalize(glm::vec3(vehiclePos.x, vehiclePos.y, vehiclePos.z) - newCamPos);
        float pitch = glm::degrees(asin(dir.y));
        float yaw = glm::degrees(atan2(dir.z, dir.x));
        camera.rotate({pitch, yaw, 0});
    }

    void setCameraFollowDistance(float distance){
        cameraFollowDistance = distance;
    }

    void setCameraFollowHeight(float height){
        cameraFollowHeight = height;
    }

    void setCameraFollowDelay(float delay){
        cameraFollowDelay = delay;
    }

    void setCameraFollowVehicle(bool shouldFollow){
        isCameraFollowingVehicle = shouldFollow;
    }

    // Temporarily public
    Camera camera;
    PlayerId id;

private:
    VehicleId vehicleId;

    PlayerKeybinds keybinds;

    bool isCameraFollowingVehicle = true;
    float cameraFollowDistance = 10.0f;
    float cameraFollowHeight = 3.0f;
    float cameraFollowDelay = 0.01f;
};