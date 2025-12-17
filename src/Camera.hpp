// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "definitions.hpp"

class Camera
{
public:
    static void move(Position3 newPosition);
    static void moveDelta(Position3 delta);

    void updateForward();

    // Temporarily public
    static Position3 position;
    static Rotation3 rotation;

    static glm::vec3 forward;

private:
    
};