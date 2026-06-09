// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../../shared/definitions.hpp"

namespace VE
{

    class Prop
    {
    public:
        Prop(PropHandle handle, ModelInstanceHandle modelInstanceHandle, Transform transform);

        [[nodiscard]] PropHandle getHandle() const;
        [[nodiscard]] glm::mat4 getModelMat() const;

        void setTransform(Transform transform);

        [[nodiscard]] ModelInstanceHandle getModelInstanceHandle() const;

        [[nodiscard]] bool hasChanges() const;
        void markChangesSaved();

    private:
        PropHandle handle;

        Transform transform;

        glm::mat4 modelMat;

        ModelInstanceHandle modelInstanceHandle;

        bool hasChanged = true;
    };

}