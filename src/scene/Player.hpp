// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Controller.hpp"

namespace VE
{
    class Player : public Controller
    {
    public:
        enum Mode
        {
            MODE_CHASE,
            MODE_FREE
        };

        Player(PlayerHandle handle, VehicleHandle vehicleHandle)
            : handle(handle), vehicleHandle(vehicleHandle)
        {
            if (vehicleHandle == VehicleHandle::INVALID)
                mode = MODE_FREE;
        }

        void tick(seconds_t dt, Position3 vehiclePosition = {}, glm::vec3 vehicleVelocityVector = {});

        void attachVehicle(VehicleHandle vehicleHandle)
        {
            this->vehicleHandle = vehicleHandle;
            mode = MODE_CHASE;
        }

        void detachVehicle()
        {
            this->vehicleHandle = VehicleHandle::INVALID;
            mode = MODE_FREE;
            vis = {};
        }

        bool hasVehicle() { return vehicleHandle != VehicleHandle::INVALID; }

        [[nodiscard]] PlayerHandle getHandle() const { return handle; }

        [[nodiscard]] VehicleInputState getVehicleInputState() const override { return vis; }

        [[nodiscard]] VehicleHandle getVehicleHandle() const override { return vehicleHandle; }

        [[nodiscard]] Position3 getPosition() const { return position; }
        [[nodiscard]] float getYaw() const { return rotation.yaw; }
        [[nodiscard]] glm::mat4 getViewMat() const;

        void setVehicleInputState(const VehicleInputState &vis) { this->vis = vis; }

        void setVehicleHandle(VehicleHandle vehicleHandle) { this->vehicleHandle = vehicleHandle; }

        void setMode(Mode mode) { this->mode = mode; }

        void setChaseDistance(float distance) { this->chaseDistance = distance; }
        void setChaseHeight(float height) { this->chaseHeight = height; }
        void setChaseDistanceDelay(float delay) { this->chaseDistanceDelay = delay; }
        void setChaseTurnDelay(float delay) { this->chaseTurnDelay = delay; }

        void setFreeSpeed(float freeSpeed) { this->freeSpeed = freeSpeed; }

        void setMinPitch(float minPitch) { this->minPitch = minPitch; }

        void setMaxPitch(float maxPitch) { this->maxPitch = maxPitch; }

    private:
        const PlayerHandle handle;

        VehicleHandle vehicleHandle;

        VehicleInputState vis;

        float pitch = 0.0f;
        float yaw = -PI;

        Position3 position{};
        Rotation3 rotation{};

        Mode mode = MODE_CHASE;
        float chaseDistance = 10.0f;
        float chaseHeight = 3.0f;
        float chaseDistanceDelay = 0.1f;
        float chaseTurnDelay = 1.0f;

        float freeSpeed = 5.0f;

        float minPitch = -1.2f;
        float maxPitch = 0.6f;
    };

}