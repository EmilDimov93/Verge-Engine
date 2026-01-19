// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Prop.hpp"

#include "../../shared/Log.hpp"

Prop::Prop(PropHandle handle, MeshInstanceHandle meshInstanceHandle, Transform transform)
: handle(handle)
{
    this->meshInstanceHandle = meshInstanceHandle;

    this->transform = transform;

    modelMat = transformToMat(transform);
}

PropHandle Prop::getHandle() const
{
    return handle;
}

glm::mat4 Prop::getModelMat()
{
    return modelMat;
}

void Prop::setTransform(Transform transform)
{
    this->transform = transform;

    modelMat = transformToMat(transform);

    hasChanged = true;
}

bool Prop::hasChanges()
{
    return hasChanged;
}

void Prop::markChangesSaved()
{
    hasChanged = false;
}
