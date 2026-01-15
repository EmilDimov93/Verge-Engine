// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../../shared/definitions.hpp"

class Prop
{
public:
    Prop(MeshInstanceHandle meshInstanceHandle, Transform transform);

    glm::mat4 getModelMat();

    void setTransform(Transform transform);

    MeshInstanceHandle meshInstanceHandle;

private:
    Transform transform;

    glm::mat4 modelMat;
};