// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "definitions.hpp"

class Camera
{
public:
    static glm::mat4 getViewMatrix();
    static glm::mat4 getProjectionMatrix();

    static void move(Position3 newPosition);
    static void moveDelta(Position3 delta);

    static void rotate(Rotation3 newRotation);
    static void rotateDelta(Rotation3 delta);

    static void update();

    // Temporarily public
    static Position3 position;
    static Rotation3 rotation;

    static glm::vec3 forward;

    static float fov;
    static float aspectRatio;

    static void init(float fov, float aspectRatio, float zFar, float zNear);

private:
};