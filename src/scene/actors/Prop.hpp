// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../../shared/definitions.hpp"

class Prop
{
public:
    Prop(PropHandle handle, MeshInstanceHandle meshInstanceHandle, Transform transform);

    PropHandle getHandle() const;
    glm::mat4 getModelMat();

    void setTransform(Transform transform);

    MeshInstanceHandle meshInstanceHandle;

private:
    PropHandle handle;

    Transform transform;

    glm::mat4 modelMat;
};