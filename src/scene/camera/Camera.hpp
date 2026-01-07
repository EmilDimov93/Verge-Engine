// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../definitions.hpp"

class Camera
{
public:
    Camera(float newFov, float newAspectRatio, float newZNear, float newZFar);
    
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();

    void move(Position3 newPosition);
    void moveDelta(Position3 delta);

    void rotate(Rotation3 newRotation);
    void rotateDelta(Rotation3 delta);

    Position3 getPosition();
    Rotation3 getRotation();

    void tick();

private:
    Position3 position = {0, 0, 0};
    Rotation3 rotation = {0, 90.0f, 0};

    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);

    float fov = -1.0f;
    float aspectRatio = -1.0f;
    float zNear = -1.0f;
    float zFar = -1.0f;

    bool isInitialized = false;
};