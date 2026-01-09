// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Trigger.hpp"

#include "../../Log.hpp"

Trigger::Trigger(uint32_t id, Transform transform, MeshInstanceId meshInstanceId, VE_STRUCT_TRIGGER_TYPE_CREATE_INFO info)
{
    this->id = id;

    this->meshInstanceId = meshInstanceId;

    if (info.hitboxShape != VE_SHAPE_UNKNOWN)
        hitboxShape = info.hitboxShape;
    else
    {
        hitboxShape = VE_SHAPE_PRISM;
        Log::add('A', 182);
    }

    if (info.hitboxSize > 0)
        hitboxSize = info.hitboxSize;
    else
    {
        hitboxSize = 1.0f;
        Log::add('A', 183);
    }

    this->transform = transform;

    isAutoDestroy = info.isAutoDestroy;

    modelMat = transformToMat(transform);
}

bool Trigger::doesActorTrigger(Position3 actorPos)
{
    Position3 triggerPos = transform.position;

    switch (hitboxShape)
    {
    case VE_SHAPE_PRISM:
    {
        if ((actorPos.x > triggerPos.x - hitboxSize / 2) && (actorPos.x < triggerPos.x + hitboxSize / 2) &&
            (actorPos.y > triggerPos.y - hitboxSize / 2) && (actorPos.y < triggerPos.y + hitboxSize / 2) &&
            (actorPos.z > triggerPos.z - hitboxSize / 2) && (actorPos.z < triggerPos.z + hitboxSize / 2))
            return true;
        break;
    }
    case VE_SHAPE_SPHERE:
    {
        float dx = actorPos.x - triggerPos.x;
        float dy = actorPos.y - triggerPos.y;
        float dz = actorPos.z - triggerPos.z;
        float radius = hitboxSize / 2;

        if (dx * dx + dy * dy + dz * dz <= radius * radius)
            return true;

        break;
    }
    case VE_SHAPE_UNKNOWN:
    default:
        Log::add('A', 182);
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

glm::mat4 Trigger::getModelMat()
{
    return modelMat;
}
