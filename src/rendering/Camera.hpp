// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../definitions.hpp"

// TODO
struct VE_STRUCT_CAMERA_INFO{

};

class Camera
{
public:
    static glm::mat4 getViewMatrix();
    static glm::mat4 getProjectionMatrix();

    static void move(Position3 newPosition);
    static void moveDelta(Position3 delta);

    static void rotate(Rotation3 newRotation);
    static void rotateDelta(Rotation3 delta);

    static Position3 getPosition();
    static Rotation3 getRotation();

    static void update();

    static void init(float newFov, float newAspectRatio, float newZNear, float newZFar);

private:
    static Position3 position;
    static Rotation3 rotation;

    static glm::vec3 forward;

    static float fov;
    static float aspectRatio;
    static float zNear;
    static float zFar;

    static bool isInitialized;
};