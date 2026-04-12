// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/definitions.hpp"

namespace VE
{

    enum HitboxShape
    {
        HITBOX_SHAPE_UNKNOWN,
        HITBOX_SHAPE_PRISM,
        HITBOX_SHAPE_SPHERE
    };

    struct TriggerTypeCreateInfo
    {
        ModelHandle modelHandle = INVALID_MODEL_HANDLE;
        HitboxShape hitboxShape = HITBOX_SHAPE_UNKNOWN;
        float hitboxSize = -1.0f;
        bool isAutoDestroy = false;
        // callback function
    };

    class Trigger
    {
    public:
        Trigger(TriggerHandle handle, Transform transform, ModelInstanceHandle modelInstanceHandle, const TriggerTypeCreateInfo &info);

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

        HitboxShape hitboxShape;
        float hitboxSize;

        bool isAutoDestroy_ = false;

        bool isMarkedForDestroy_ = false;

        glm::mat4 modelMat;

        // callback function
    };

}