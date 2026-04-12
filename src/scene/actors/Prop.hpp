// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../../shared/definitions.hpp"

namespace VE
{

    class Prop
    {
    public:
        Prop(PropHandle handle, ModelInstanceHandle modelInstanceHandle, Transform transform);

        PropHandle getHandle() const;
        glm::mat4 getModelMat() const;

        void setTransform(Transform transform);

        ModelInstanceHandle getModelInstanceHandle() const;

        bool hasChanges() const;
        void markChangesSaved();

    private:
        PropHandle handle;

        Transform transform;

        glm::mat4 modelMat;

        ModelInstanceHandle modelInstanceHandle;

        bool hasChanged = true;
    };

}