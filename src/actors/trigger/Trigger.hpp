// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../definitions.hpp"

enum VE_SHAPE
{
    VE_SHAPE_UNKNOWN,
    VE_SHAPE_PRISM,
    VE_SHAPE_SPHERE
};

struct VE_STRUCT_TRIGGER_TYPE_CREATE_INFO
{
    int32_t meshIndex = -1;
    VE_SHAPE hitboxShape = VE_SHAPE_UNKNOWN;
    float hitboxSize = -1.0f;
    bool isAutoDestroy = false;
    // callback function
};

class Trigger
{
public:
    Trigger(uint32_t id, Transform transform, uint32_t meshInstanceIndex, VE_STRUCT_TRIGGER_TYPE_CREATE_INFO info);

    bool doesActorTrigger(Position3 actorPos);

    uint32_t getId() const;
    bool getIsAutoDestroy() const;

    void markForDestroy();
    bool getIsMarkedForDestroy() const;

    glm::mat4 getModelMat();

private:
    uint32_t id;

    Transform transform;

    uint32_t meshInstanceIndex;

    VE_SHAPE hitboxShape;
    float hitboxSize;

    bool isAutoDestroy;

    bool isMarkedForDestroy = false;

    glm::mat4 modelMat;

    // callback function
};