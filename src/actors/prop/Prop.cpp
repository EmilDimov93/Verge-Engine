// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Prop.hpp"

#include "../../Log.hpp"

Prop::Prop(uint32_t meshIndex, Position3 position, Rotation3 rotation)
{
    if (meshIndex >= 0)
    {
        this->meshIndex = meshIndex;
    }
    else
    {
        this->meshIndex = -1;
        Log::add('A', 160);
    }

    this->position = position;

    this->rotation = rotation;

    modelMat = glm::mat4(1.0f);

    modelMat = glm::translate(modelMat, glm::vec3(position.x, position.y, position.z));

    modelMat = glm::rotate(modelMat, (float)rotation.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::rotate(modelMat, (float)rotation.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMat = glm::rotate(modelMat, (float)rotation.roll, glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::mat4 Prop::getModelMat()
{
    return modelMat;
}
