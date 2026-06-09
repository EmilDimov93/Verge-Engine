// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

namespace VE
{

    class Camera
    {
    public:
        [[nodiscard]] glm::mat4 getViewMat() const;

        void move(Position3 newPosition);
        void rotate(Rotation3 newRotation);

        [[nodiscard]] Position3 getPosition() const;
        [[nodiscard]] Rotation3 getRotation() const;

    private:
        Position3 position{};
        Rotation3 rotation{};
    };

}