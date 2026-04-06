// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/definitions.hpp"

enum VEShape
{
    VE_SHAPE_UNKNOWN,
    VE_SHAPE_PRISM,
    VE_SHAPE_SPHERE
};

struct VETriggerTypeCreateInfo
{
    ModelHandle modelHandle = INVALID_MODEL_HANDLE;
    VEShape hitboxShape = VE_SHAPE_UNKNOWN;
    float hitboxSize = -1.0f;
    bool isAutoDestroy = false;
    // callback function
};

class Trigger
{
public:
    Trigger(TriggerHandle handle, Transform transform, ModelInstanceHandle modelInstanceHandle, const VETriggerTypeCreateInfo& info);

    TriggerHandle getHandle() const;
    glm::mat4 getModelMat() const;

    ModelInstanceHandle getModelInstanceHandle() const;

    bool doesActorTrigger(Position3 actorPos) const;

    bool isAutoDestroy() const;
    void markForDestroy();
    bool isMarkedForDestroy() const;

private:
    TriggerHandle handle;

    Transform transform;

    ModelInstanceHandle modelInstanceHandle;

    VEShape hitboxShape;
    float hitboxSize;

    bool isAutoDestroy_ = false;

    bool isMarkedForDestroy_ = false;

    glm::mat4 modelMat;

    // callback function
};