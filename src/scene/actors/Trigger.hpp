// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/definitions.hpp"

enum VE_SHAPE
{
    VE_SHAPE_UNKNOWN,
    VE_SHAPE_PRISM,
    VE_SHAPE_SPHERE
};

struct VE_STRUCT_TRIGGER_TYPE_CREATE_INFO
{
    MeshHandle meshHandle = INVALID_MESH_HANDLE;
    VE_SHAPE hitboxShape = VE_SHAPE_UNKNOWN;
    float hitboxSize = -1.0f;
    bool isAutoDestroy = false;
    // callback function
};

class Trigger
{
public:
    Trigger(TriggerHandle handle, Transform transform, MeshInstanceHandle meshInstanceHandle, VE_STRUCT_TRIGGER_TYPE_CREATE_INFO info);

    TriggerHandle getHandle() const;
    glm::mat4 getModelMat();

    bool doesActorTrigger(Position3 actorPos);

    bool getIsAutoDestroy() const;
    void markForDestroy();
    bool getIsMarkedForDestroy() const;

private:
    TriggerHandle handle;

    Transform transform;

    MeshInstanceHandle meshInstanceHandle;

    VE_SHAPE hitboxShape;
    float hitboxSize;

    bool isAutoDestroy;

    bool isMarkedForDestroy = false;

    glm::mat4 modelMat;

    // callback function
};