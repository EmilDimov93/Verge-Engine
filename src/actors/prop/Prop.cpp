// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Prop.hpp"

#include "../../Log.hpp"

Prop::Prop(uint32_t meshIndex, Transform transform)
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

    this->transform = transform;

    modelMat = transformToMat(transform);
}

glm::mat4 Prop::getModelMat()
{
    return modelMat;
}
