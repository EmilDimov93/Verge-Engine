// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/definitions.hpp"

using TriggerId = uint64_t;

enum VE_SHAPE
{
    VE_SHAPE_UNKNOWN,
    VE_SHAPE_PRISM,
    VE_SHAPE_SPHERE
};

struct VE_STRUCT_TRIGGER_TYPE_CREATE_INFO
{
    MeshId meshId = INVALID_MESH_ID;
    VE_SHAPE hitboxShape = VE_SHAPE_UNKNOWN;
    float hitboxSize = -1.0f;
    bool isAutoDestroy = false;
    // callback function
};

class Trigger
{
public:
    Trigger(TriggerId id, Transform transform, MeshInstanceId meshInstanceId, VE_STRUCT_TRIGGER_TYPE_CREATE_INFO info);

    bool doesActorTrigger(Position3 actorPos);

    TriggerId getId() const;
    bool getIsAutoDestroy() const;

    void markForDestroy();
    bool getIsMarkedForDestroy() const;

    glm::mat4 getModelMat();

private:
    TriggerId id;

    Transform transform;

    MeshInstanceId meshInstanceId;

    VE_SHAPE hitboxShape;
    float hitboxSize;

    bool isAutoDestroy;

    bool isMarkedForDestroy = false;

    glm::mat4 modelMat;

    // callback function
};