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
        Log::add('A', 180);
    }

    if (info.meshIndex >= 0)
        meshIndex = info.meshIndex;
    else
    {
        Log::add('A', 181);
    }

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

    this->position = position;

    isAutoDestroy = info.isAutoDestroy;

    modelMat = glm::mat4(1.0f);

    modelMat = glm::translate(modelMat, glm::vec3(position.x, position.y, position.z));

    modelMat = glm::rotate(modelMat, (float)rotation.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::rotate(modelMat, (float)rotation.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMat = glm::rotate(modelMat, (float)rotation.roll, glm::vec3(0.0f, 0.0f, 1.0f));
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
