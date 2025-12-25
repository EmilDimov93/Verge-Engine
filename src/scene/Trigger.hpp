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
    int32_t id = -1;
    int32_t meshIndex = -1;
    VE_SHAPE hitboxShape = VE_SHAPE_UNKNOWN;
    float hitboxSize = -1.0f;
    bool destroyOnTrigger = false;
    // callback function
};

class Trigger
{
public:
    Trigger(VE_STRUCT_TRIGGER_TYPE_CREATE_INFO info, Position3 position);

    bool doesActorTrigger(Position3 actorPosition);

private:
    int32_t id;

    Position3 position;
    int32_t meshIndex;

    VE_SHAPE hitboxShape;
    float hitboxSize;

    bool destroyOnTrigger;

    // callback function
};