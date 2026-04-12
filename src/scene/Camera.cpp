// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Camera.hpp"

#include "../shared/Log.hpp"

namespace VE
{

    glm::mat4 Camera::getViewMat() const
    {
        glm::vec3 forward;
        forward.x = cos(rotation.pitch) * sin(rotation.yaw);
        forward.y = -sin(rotation.pitch);
        forward.z = cos(rotation.pitch) * cos(rotation.yaw);
        forward = glm::normalize(forward);

        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));
        up = up * (float)cos(rotation.roll) + right * (float)sin(rotation.roll);

        glm::vec3 pos(position.x, position.y, position.z);
        return glm::lookAt(pos, glm::vec3(position.x, position.y, position.z) + forward, up);
    }

    void Camera::move(Position3 newPosition)
    {
        position = newPosition;
    }

    void Camera::rotate(Rotation3 newRotation)
    {
        rotation = newRotation;
    }

    Position3 Camera::getPosition() const
    {
        return position;
    }

    Rotation3 Camera::getRotation() const
    {
        return rotation;
    }

}