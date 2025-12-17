// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Camera.hpp"

Position3 Camera::position = {0, 0, 0};
Rotation3 Camera::rotation = {0, -90.0f, 0};
glm::vec3 Camera::forward = glm::vec3(0.0f, 0.0f, -1.0f);

float Camera::fov = 45.0f;
float Camera::aspectRatio = 0;

glm::mat4 Camera::getViewMatrix(){
    return glm::lookAt(glm::vec3(position.x, position.y, position.z), glm::vec3(position.x + forward.x, position.y + forward.y, position.z + forward.z), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getProjectionMatrix(){
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
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

void Camera::update() {
    forward.x = cos(glm::radians(rotation.yaw)) * cos(glm::radians(rotation.pitch));
    forward.y = sin(glm::radians(rotation.pitch));
    forward.z = sin(glm::radians(rotation.yaw)) * cos(glm::radians(rotation.pitch));
    forward = glm::normalize(forward);
}

void Camera::init(float fov, float aspectRatio, float zFar, float zNear)
{
}
