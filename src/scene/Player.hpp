// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Controller.hpp"

#include "Camera.hpp"

class Player : public Controller
{
public:
    Player(PlayerHandle handle, VehicleHandle vehicleHandle, const VE_STRUCT_CAMERA_CREATE_INFO &cameraInfo)
        : handle(handle), camera(cameraInfo)
    {
        this->vehicleHandle = vehicleHandle;
    }

    void setVIS(const VehicleInputState &vis){
        this->vis = vis;
    }

    VehicleInputState getVehicleInputState() const override
    {
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

        Position3 vehiclePos = vehicleTransform.position;

        camPos = {camera.getPosition().x, camera.getPosition().y, camera.getPosition().z};
        glm::vec3 targetCamPos = {vehiclePos.x + sin(cameraYaw) * cameraFollowDistance, vehiclePos.y + sin(cameraPitch) * cameraFollowDistance + currCameraHeight, vehiclePos.z + cos(cameraYaw) * cos(cameraPitch) * cameraFollowDistance};
        camPos = glm::mix(camPos, targetCamPos, 1.0f - std::exp(-float(dt) * 10.0f));
        camera.move({camPos.x, camPos.y, camPos.z});

        float targetYaw = atan2(vehicleVelocityVector.x, vehicleVelocityVector.z) - PI;
        if (glm::length(vehicleVelocityVector) < 1.0f)
            targetYaw = cameraYaw;

        // Temporary: camera movement disabled
        cameraYaw += wrapRadToPi(targetYaw - cameraYaw) * cameraFollowDelay;// + (keybinds.moveCameraRight.getValue() - keybinds.moveCameraLeft.getValue()) * PI * dt;
        //cameraPitch += (keybinds.moveCameraUp.getValue() - keybinds.moveCameraDown.getValue()) * PI * dt;
        
        cameraPitch = clamp(cameraPitch, minCameraPitch, maxCameraPitch);

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
        if (volume >= 0 && volume <= 1.0f)
            this->volume = volume;
    }

    float getVolume() const
    {
        return volume;
    }

    void setRenderDistance(float renderDistance)
    {
        if (renderDistance >= 0)
            camera.setZFar(renderDistance);
    }

    void setMinCameraPitch(float minCameraPitch)
    {
        this->minCameraPitch = minCameraPitch;
    }

    void setMaxCameraPitch(float maxCameraPitch)
    {
        this->maxCameraPitch = maxCameraPitch;
    }

private:
    const PlayerHandle handle;

    VehicleHandle vehicleHandle;

    Camera camera;

    VehicleInputState vis;

    float cameraPitch = 0.0f;
    float cameraYaw = -PI;
    glm::vec3 camPos;

    bool isCameraFollowingVehicle = true;
    float cameraFollowDistance = 10.0f;
    float cameraFollowHeight = 3.0f;
    float cameraFollowDelay = 0.01f;

    float minCameraPitch = -1.2f;
    float maxCameraPitch = 0.6f;

    float volume = 1.0f;

    float wrapRadToPi(float angleRad)
    {
        angleRad = std::fmod(angleRad + PI, 2.0f * PI);
        if (angleRad < 0.0f)
            angleRad += 2.0f * PI;
        return angleRad - PI;
    }
};