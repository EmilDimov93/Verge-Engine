// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Trigger.hpp"

#include "../Log.hpp"

Trigger::Trigger(VE_STRUCT_TRIGGER_TYPE_CREATE_INFO info, Position3 position)
{
    if (info.id != -1)
        id = info.id;
    else
    {
        // should be error
        Log::add('O', 101);
    }

    if (info.meshIndex != -1)
        meshIndex = info.meshIndex;
    else
    {
        // should be error
        Log::add('O', 101);
    }

    if (info.hitboxShape != VE_SHAPE_UNKNOWN)
        hitboxShape = info.hitboxShape;
    else
    {
        hitboxShape = VE_SHAPE_PRISM;
        Log::add('O', 101);
    }

    if (info.hitboxSize > 0)
        hitboxSize = info.hitboxSize;
    else
    {
        hitboxSize = 1.0f;
        Log::add('O', 101);
    }

    this->position = position;

    destroyOnTrigger = info.destroyOnTrigger;
}

bool Trigger::doesActorTrigger(Position3 actorPosition)
{
    switch (hitboxShape)
    {
    case VE_SHAPE_PRISM:
    {
        if ((actorPosition.x > position.x - hitboxSize / 2) && (actorPosition.x < position.x + hitboxSize / 2) &&
            (actorPosition.y > position.y - hitboxSize / 2) && (actorPosition.y < position.y + hitboxSize / 2) &&
            (actorPosition.z > position.z - hitboxSize / 2) && (actorPosition.z < position.z + hitboxSize / 2))
            return true;
        break;
    }
    case VE_SHAPE_SPHERE:
    case VE_SHAPE_LINE:
    case VE_SHAPE_UNKNOWN:
    default:
        break;
    }

    return false;
}