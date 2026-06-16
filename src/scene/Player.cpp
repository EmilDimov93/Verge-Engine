// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Player.hpp"

namespace VE
{
    void Player::tick(milliseconds_t dt, Position3 vehiclePosition, glm::vec3 vehicleVelocityVector)
    {
        if (mode == MODE_CHASE)
        {
            float targetYaw = atan2(vehicleVelocityVector.x, vehicleVelocityVector.z) - PI;
            if (glm::length(vehicleVelocityVector) < 1.0f)
                targetYaw = yaw;

            yaw += wrapRadToPi(targetYaw - yaw) * (1.0f - std::exp(-float(dt) / chaseTurnDelay)) + (vis.moveCameraRight - vis.moveCameraLeft) * PI * dt;
            yaw = wrapRadToPi(yaw);

            pitch += (vis.moveCameraUp - vis.moveCameraDown) * PI * dt;
            pitch = clamp(pitch, minPitch, maxPitch);

            glm::vec3 targetPosition = {vehiclePosition.x + sin(yaw) * chaseDistance, vehiclePosition.y + sin(pitch) * chaseDistance + chaseHeight, vehiclePosition.z + cos(yaw) * cos(pitch) * chaseDistance};
            position = glm::mix(position, targetPosition, 1.0f - std::exp(-float(dt) / chaseDistanceDelay));

            glm::vec3 dir = glm::normalize(vehiclePosition - position);
            rotation = {-asin(dir.y), atan2(dir.x, dir.z), 0};
        }
        else
        {
            rotation.yaw += (vis.moveCameraLeft - vis.moveCameraRight) * PI * dt;
            rotation.pitch += (vis.moveCameraDown - vis.moveCameraUp) * PI * dt;

            rotation.pitch = clamp(rotation.pitch, minPitch, maxPitch);

            rotation = {rotation.pitch, rotation.yaw, 0.f};

            float forwardBackward = (vis.throttle - vis.brake) * dt;
            glm::vec3 forward = {cosf(rotation.pitch) * sinf(rotation.yaw), -sinf(rotation.pitch), cosf(rotation.pitch) * cosf(rotation.yaw)};
            glm::vec3 forwardScaled = forward * forwardBackward;

            float leftRight = vis.steer * dt;
            glm::vec3 right = {forward.z, 0.f, -forward.x};
            glm::vec3 rightScaled = right * leftRight;

            glm::vec3 delta = (forwardScaled + rightScaled) * freeSpeed;

            position += delta;
        }
    }

    glm::mat4 Player::getViewMat() const
    {
        glm::vec3 forward(cos(rotation.pitch) * sin(rotation.yaw), -sin(rotation.pitch), cos(rotation.pitch) * cos(rotation.yaw));
        glm::vec3 right = glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 up = glm::cross(right, forward) * cos(rotation.roll) + right * sin(rotation.roll);

        return glm::lookAt(position, position + forward, up);
    }
}