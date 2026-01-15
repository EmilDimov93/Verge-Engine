// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Prop.hpp"

#include "../../shared/Log.hpp"

Prop::Prop(MeshInstanceHandle meshInstanceHandle, Transform transform)
{
    this->meshInstanceHandle = meshInstanceHandle;

    this->transform = transform;

    modelMat = transformToMat(transform);
}

glm::mat4 Prop::getModelMat()
{
    return modelMat;
}

void Prop::setTransform(Transform transform)
{
    this->transform = transform;

    modelMat = transformToMat(transform);
}
