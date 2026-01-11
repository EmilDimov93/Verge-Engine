// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../../shared/definitions.hpp"

class Prop
{
public:
    Prop(MeshInstanceId meshInstanceId, Transform transform);

    glm::mat4 getModelMat();

    void setTransform(Transform transform);

    MeshInstanceId meshInstanceId;

private:
    Transform transform;

    glm::mat4 modelMat;
};