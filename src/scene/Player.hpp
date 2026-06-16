// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Controller.hpp"

#include "Camera.hpp"

namespace VE
{
    enum CameraMode
    {
        CAMERA_MODE_CHASE,
        CAMERA_MODE_FREE
    };

    class Player : public Controller
    {
    public:
        Player(PlayerHandle handle, VehicleHandle vehicleHandle)
            : handle(handle), vehicleHandle(vehicleHandle)
        {
            if(vehicleHandle == VehicleHandle::INVALID)
                cameraMode = CAMERA_MODE_FREE;
        }

        void attachVehicle(VehicleHandle vehicleHandle)
        {
            this->vehicleHandle = vehicleHandle;
            cameraMode = CAMERA_MODE_CHASE;
        }

        void detachVehicle()
        {
            this->vehicleHandle = VehicleHandle::INVALID;
            cameraMode = CAMERA_MODE_FREE;
        }

        bool hasVehicle()
        {
            return vehicleHandle != VehicleHandle::INVALID;
        }

        void setVehicleInputState(const VehicleInputState &vis)  { this->vis = vis; }
        [[nodiscard]] VehicleInputState getVehicleInputState() const override { return vis; }

        void setVehicleHandle(VehicleHandle vehicleHandle) { this->vehicleHandle = vehicleHandle; }
        [[nodiscard]] VehicleHandle getVehicleHandle() const override { return vehicleHandle; }

        [[nodiscard]] PlayerHandle getHandle() const { return handle; }

        void updateCameraChaseMode(milliseconds_t dt, Position3 vehiclePosition, glm::vec3 vehicleVelocityVector);
        void updateCameraFreeMode(milliseconds_t dt);

        [[nodiscard]] glm::mat4 getCameraViewMat() const { return camera.getViewMat(); }

        [[nodiscard]] Position3 getCameraPosition() const { return camera.getPosition(); }

        [[nodiscard]] float getCameraYaw() const { return camera.getRotation().yaw; }

        void setCameraChaseDistance(float distance) { this->cameraChaseDistance = distance; }

        void setCameraChaseHeight(float height) { this->cameraChaseHeight = height; }

        void setCameraChaseDistanceDelay(float delay) { this->cameraChaseDistanceDelay = delay; }
        void setCameraChaseTurnDelay(float delay) { this->cameraChaseTurnDelay = delay; }

        void setCameraMode(CameraMode mode) { this->cameraMode = mode; }

        void setMinCameraPitch(float minCameraPitch) { this->minCameraPitch = minCameraPitch; }

        void setMaxCameraPitch(float maxCameraPitch) { this->maxCameraPitch = maxCameraPitch; }

        void setFreeCameraSpeed(float freeCameraSpeed) { this->cameraFreeSpeed = freeCameraSpeed; }

    private:
        const PlayerHandle handle;

        VehicleHandle vehicleHandle;

        Camera camera;

        VehicleInputState vis;

        float cameraPitch = 0.0f;
        float cameraYaw = -PI;

        CameraMode cameraMode = CAMERA_MODE_CHASE;
        float cameraChaseDistance = 10.0f;
        float cameraChaseHeight = 3.0f;
        float cameraChaseDistanceDelay = 0.1f;
        float cameraChaseTurnDelay = 1.0f;
        float cameraFreeSpeed = 5.0f;

        float minCameraPitch = -1.2f;
        float maxCameraPitch = 0.6f;
    };

}