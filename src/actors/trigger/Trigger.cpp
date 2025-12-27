// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Trigger.hpp"

#include "../../Log.hpp"

Trigger::Trigger(uint32_t id, Position3 position, VE_STRUCT_TRIGGER_TYPE_CREATE_INFO info)
{
    if (id >= 0)
        this->id = id;
    else
    {
        // should be error
        Log::add('O', 101);
    }

    if (info.meshIndex >= 0)
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

    isAutoDestroy = info.isAutoDestroy;
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
    {
        float dx = actorPosition.x - position.x;
        float dy = actorPosition.y - position.y;
        float dz = actorPosition.z - position.z;
        float radius = hitboxSize / 2;

        if (dx*dx + dy*dy + dz*dz <= radius * radius)
            return true;

        break;
    }
    case VE_SHAPE_UNKNOWN:
    default:
        Log::add('O', 101);
        break;
    }

    return false;
}

uint32_t Trigger::getId() const
{
    return id;
}

bool Trigger::getIsAutoDestroy() const
{
    return isAutoDestroy;
}

void Trigger::markForDestroy()
{
    isMarkedForDestroy = true;
}

bool Trigger::getIsMarkedForDestroy() const
{
    return isMarkedForDestroy;
}
