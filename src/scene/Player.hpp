// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Controller.hpp"

#include "Camera.hpp"

class Player : public Controller
{
public:
    Player(PlayerHandle handle, VehicleHandle vehicleHandle)
        : handle(handle), vehicleHandle(vehicleHandle)
    {
    }

    void setVehicleInputState(const VehicleInputState &vis);
    VehicleInputState getVehicleInputState() const override;

    void setVehicleHandle(VehicleHandle vehicleHandle);
    VehicleHandle getVehicleHandle() const override;

    PlayerHandle getHandle() const;

    void updateCamera(ve_time_t dt, Transform vehicleTransform, glm::vec3 vehicleVelocityVector);

    void setCameraFollowDistance(float distance);

    void setCameraFollowHeight(float height);

    void setCameraFollowDelay(float delay);

    void setCameraFollowVehicle(bool shouldFollow);

    glm::mat4 getCameraViewMat() const;

    Position3 getCameraPosition() const;

    float getCameraYaw() const;

    void setMinCameraPitch(float minCameraPitch);

    void setMaxCameraPitch(float maxCameraPitch);

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
};