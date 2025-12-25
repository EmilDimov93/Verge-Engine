// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../definitions.hpp"

enum VE_SHAPE{
    VE_SHAPE_UNKNOWN,
    VE_SHAPE_PRISM,
    VE_SHAPE_SPHERE,
    VE_SHAPE_LINE
};

struct VE_STRUCT_TRIGGER_TYPE_CREATE_INFO
{
    int32_t meshIndex = -1;
    VE_SHAPE hitboxShape = VE_SHAPE_UNKNOWN;
    float hitboxSize = -1.0f;
    bool destroyOnTrigger = false;
    // callback function
};

class Trigger
{
public:
    Trigger(uint32_t id, Position3 position, VE_STRUCT_TRIGGER_TYPE_CREATE_INFO info);

    bool doesActorTrigger(Position3 actorPosition);

    uint32_t getId();

private:
    uint32_t id;

    Position3 position;
    uint32_t meshIndex;

    VE_SHAPE hitboxShape;
    float hitboxSize;

    bool destroyOnTrigger;

    // callback function
};