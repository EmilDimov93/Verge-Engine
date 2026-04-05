// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Prop.hpp"

#include "../../shared/Log.hpp"

Prop::Prop(PropHandle handle, ModelInstanceHandle modelInstanceHandle, Transform transform)
    : handle(handle)
{
    this->modelInstanceHandle = modelInstanceHandle;

    this->transform = transform;

    modelMat = transform.toMat();
}

PropHandle Prop::getHandle() const
{
    return handle;
}

glm::mat4 Prop::getModelMat() const
{
    return modelMat;
}

void Prop::setTransform(Transform transform)
{
    this->transform = transform;

    modelMat = transform.toMat();

    hasChanged = true;
}

ModelInstanceHandle Prop::getModelInstanceHandle() const
{
    return modelInstanceHandle;
}

bool Prop::hasChanges() const
{
    return hasChanged;
}

void Prop::markChangesSaved()
{
    hasChanged = false;
}
