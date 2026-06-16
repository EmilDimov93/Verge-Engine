// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Player.hpp"

namespace VE
{
    void Player::updateCameraChaseMode(milliseconds_t dt, Position3 vehiclePosition, glm::vec3 vehicleVelocityVector)
    {
        float targetYaw = atan2(vehicleVelocityVector.x, vehicleVelocityVector.z) - PI;
        if (glm::length(vehicleVelocityVector) < 1.0f)
            targetYaw = cameraYaw;

        cameraYaw += wrapRadToPi(targetYaw - cameraYaw) * (1.0f - std::exp(-float(dt) / cameraChaseTurnDelay)) + (vis.moveCameraRight - vis.moveCameraLeft) * PI * dt;
        cameraYaw = wrapRadToPi(cameraYaw);

        cameraPitch += (vis.moveCameraUp - vis.moveCameraDown) * PI * dt;
        cameraPitch = clamp(cameraPitch, minCameraPitch, maxCameraPitch);

        Position3 position = camera.getPosition();
        glm::vec3 targetPosition = {vehiclePosition.x + sin(cameraYaw) * cameraChaseDistance, vehiclePosition.y + sin(cameraPitch) * cameraChaseDistance + cameraChaseHeight, vehiclePosition.z + cos(cameraYaw) * cos(cameraPitch) * cameraChaseDistance};
        camera.move(glm::mix(position, targetPosition, 1.0f - std::exp(-float(dt) / cameraChaseDistanceDelay)));

        glm::vec3 dir = glm::normalize(vehiclePosition - position);
        camera.rotate({-asin(dir.y), atan2(dir.x, dir.z), 0});
    }

    void Player::updateCameraFreeMode(milliseconds_t dt)
    {
        Rotation3 rotation = camera.getRotation();

        rotation.yaw += (vis.moveCameraLeft - vis.moveCameraRight) * PI * dt;
        rotation.pitch += (vis.moveCameraDown - vis.moveCameraUp) * PI * dt;

        rotation.pitch = clamp(rotation.pitch, minCameraPitch, maxCameraPitch);

        camera.rotate({rotation.pitch, rotation.yaw, 0.});

        Position3 position = camera.getPosition();

        float cameraForwardBackward = (vis.throttle - vis.brake) * dt;
        glm::vec3 forward = {cosf(rotation.pitch) * sinf(rotation.yaw), -sinf(rotation.pitch), cosf(rotation.pitch) * cosf(rotation.yaw)};
        glm::vec3 forwardScaled = forward * cameraForwardBackward;

        float cameraLeftRight = vis.steer * dt;
        glm::vec3 right = {forward.z, 0.f, -forward.x};
        glm::vec3 rightScaled = right * cameraLeftRight;

        glm::vec3 delta = (forwardScaled + rightScaled) * cameraFreeSpeed;

        camera.move(position + delta);
    }
}