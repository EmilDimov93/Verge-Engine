// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../../definitions.hpp"

class Prop
{
public:
    Prop(uint32_t meshIndex, Transform transform);

    glm::mat4 getModelMat();

private:
    Transform transform;

    uint32_t meshIndex;

    glm::mat4 modelMat;
};