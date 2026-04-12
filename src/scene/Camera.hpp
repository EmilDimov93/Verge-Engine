// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

namespace VE
{

    class Camera
    {
    public:
        glm::mat4 getViewMat() const;

        void move(Position3 newPosition);
        void rotate(Rotation3 newRotation);

        Position3 getPosition() const;
        Rotation3 getRotation() const;

    private:
        Position3 position;
        Rotation3 rotation;
    };

}