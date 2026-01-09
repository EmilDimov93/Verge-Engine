// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../../definitions.hpp"

class Prop
{
public:
    Prop(MeshInstanceId meshInstanceId, Transform transform);

    glm::mat4 getModelMat();

private:
    Transform transform;

    MeshInstanceId meshInstanceId;

    glm::mat4 modelMat;
};