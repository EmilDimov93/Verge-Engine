// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Player.hpp"

void Player::setVehicleInputState(const VehicleInputState &vis)
{
    this->vis = vis;
}

VehicleInputState Player::getVehicleInputState() const
{
    return vis;
}

void Player::setVehicleHandle(VehicleHandle vehicleHandle)
{
    this->vehicleHandle = vehicleHandle;
}

VehicleHandle Player::getVehicleHandle() const
{
    return vehicleHandle;
}

PlayerHandle Player::getHandle() const
{
    return handle;
}

void Player::updateCamera(ve_time_t dt, Transform vehicleTransform, glm::vec3 vehicleVelocityVector)
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

    cameraYaw += wrapRadToPi(targetYaw - cameraYaw) * cameraFollowDelay + (vis.moveCameraRight - vis.moveCameraLeft) * PI * dt;
    cameraPitch += (vis.moveCameraUp - vis.moveCameraDown) * PI * dt;

    cameraPitch = clamp(cameraPitch, minCameraPitch, maxCameraPitch);

    glm::vec3 dir = glm::normalize(glm::vec3(vehiclePos.x, vehiclePos.y, vehiclePos.z) - camPos);
    camera.rotate({asin(dir.y), atan2(dir.z, dir.x), 0});
}

void Player::setCameraFollowDistance(float distance)
{
    cameraFollowDistance = distance;
}

void Player::setCameraFollowHeight(float height)
{
    cameraFollowHeight = height;
}

void Player::setCameraFollowDelay(float delay)
{
    cameraFollowDelay = delay;
}

void Player::setCameraFollowVehicle(bool shouldFollow)
{
    isCameraFollowingVehicle = shouldFollow;
}

glm::mat4 Player::getCameraProjectionMat() const
{
    return camera.getProjectionMat();
}

glm::mat4 Player::getCameraViewMat() const
{
    return camera.getViewMat();
}

Position3 Player::getCameraPosition() const
{
    return camera.getPosition();
}

float Player::getCameraYaw() const
{
    return camera.getRotation().yaw;
}

void Player::setRenderDistance(float renderDistance)
{
    if (renderDistance >= 0)
        camera.setZFar(renderDistance);
}

void Player::setMinCameraPitch(float minCameraPitch)
{
    this->minCameraPitch = minCameraPitch;
}

void Player::setMaxCameraPitch(float maxCameraPitch)
{
    this->maxCameraPitch = maxCameraPitch;
}