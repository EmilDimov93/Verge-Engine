// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Trigger.hpp"

#include "../../shared/Log.hpp"

namespace VE
{

    Trigger::Trigger(TriggerHandle handle, Transform transform, ModelInstanceHandle modelInstanceHandle, const TriggerTypeCreateInfo &info, const std::function<void()>& callback)
        : handle(handle), callback_(callback)
    {
        this->modelInstanceHandle = modelInstanceHandle;

        if (info.hitboxShape != HITBOX_SHAPE_UNKNOWN)
        {
            hitboxShape = info.hitboxShape;
        }
        else
        {
            hitboxShape = HITBOX_SHAPE_PRISM;
            Log::add('A', 182);
        }

        if (info.hitboxSize > 0)
        {
            hitboxSize = info.hitboxSize;
        }
        else
        {
            hitboxSize = 1.0f;
            Log::add('A', 183);
        }

        this->transform = transform;

        isAutoDestroy_ = info.isAutoDestroy;

        modelMat = transform.toMat();
    }

    bool Trigger::doesActorTrigger(Position3 actorPos) const
    {
        Position3 triggerPos = transform.position;

        switch (hitboxShape)
        {
        case HITBOX_SHAPE_PRISM:
        {
            if ((actorPos.x > triggerPos.x - hitboxSize / 2) && (actorPos.x < triggerPos.x + hitboxSize / 2) &&
                (actorPos.y > triggerPos.y - hitboxSize / 2) && (actorPos.y < triggerPos.y + hitboxSize / 2) &&
                (actorPos.z > triggerPos.z - hitboxSize / 2) && (actorPos.z < triggerPos.z + hitboxSize / 2))
                return true;
            break;
        }
        case HITBOX_SHAPE_SPHERE:
        {
            glm::vec3 delta = actorPos - triggerPos;
            float radius = hitboxSize / 2;

            if (glm::length(delta) <= radius * radius)
                return true;

            break;
        }
        case HITBOX_SHAPE_UNKNOWN:
        default:
            Log::add('A', 182);
            break;
        }

        return false;
    }

    TriggerHandle Trigger::getHandle() const
    {
        return handle;
    }

    glm::mat4 Trigger::getModelMat() const
    {
        return modelMat;
    }

    ModelInstanceHandle Trigger::getModelInstanceHandle() const
    {
        return modelInstanceHandle;
    }

    bool Trigger::isAutoDestroy() const
    {
        return isAutoDestroy_;
    }

    void Trigger::markForDestroy()
    {
        isMarkedForDestroy_ = true;
    }

    bool Trigger::isMarkedForDestroy() const
    {
        return isMarkedForDestroy_;
    }

}