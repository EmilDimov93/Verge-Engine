// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Camera.hpp"

#include "../shared/Log.hpp"

Camera::Camera(const VE_STRUCT_CAMERA_CREATE_INFO &info)
{
    if (isInitialized)
    {
        Log::add('K', 100);
        return;
    }

    if (info.aspectRatio <= 0)
        Log::add('K', 202);

    if (info.fov <= 0 || info.fov >= 180)
        Log::add('K', 201);

    if (info.zNear <= 0)
        Log::add('K', 203);

    if (info.zFar <= info.zNear)
        Log::add('K', 204);

    fov = info.fov;
    aspectRatio = info.aspectRatio;
    zNear = info.zNear;
    zFar = info.zFar;

    isInitialized = true;
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(glm::vec3(position.x, position.y, position.z), glm::vec3(position.x + forward.x, position.y + forward.y, position.z + forward.z), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getProjectionMatrix()
{
    if (!isInitialized)
    {
        Log::add('K', 200);
    }

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);
    projectionMatrix[1][1] *= -1;
    return projectionMatrix;
}

void Camera::move(Position3 newPosition)
{
    position = newPosition;
}

void Camera::moveDelta(Position3 delta)
{
    position.x += delta.x;
    position.y += delta.y;
    position.z += delta.z;
}

void Camera::rotate(Rotation3 newRotation)
{
    rotation = newRotation;
}

void Camera::rotateDelta(Rotation3 delta)
{
    rotation.pitch += delta.pitch;
    rotation.yaw += delta.yaw;
    rotation.roll += delta.roll;
}

Position3 Camera::getPosition()
{
    return position;
}

Rotation3 Camera::getRotation()
{
    return rotation;
}

void Camera::tick()
{
    forward.x = cos(glm::radians(rotation.yaw)) * cos(glm::radians(rotation.pitch));
    forward.y = sin(glm::radians(rotation.pitch));
    forward.z = sin(glm::radians(rotation.yaw)) * cos(glm::radians(rotation.pitch));
    forward = glm::normalize(forward);
}
