// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

struct VE_STRUCT_CAMERA_CREATE_INFO
{
    float aspectRatio = 1.0f;
    float fov = 60.0f;
    float zNear = 0.01f;
    float zFar = 1000.0f;
};

class Camera
{
public:
    Camera(const VE_STRUCT_CAMERA_CREATE_INFO &info);

    glm::mat4 getViewMat() const;
    glm::mat4 getProjectionMat() const;

    void move(Position3 newPosition);
    void moveDelta(Position3 delta);

    void rotate(Rotation3 newRotation);
    void rotateDelta(Rotation3 delta);

    Position3 getPosition() const;
    Rotation3 getRotation() const;

    void setZFar(float zFar);

private:
    Position3 position;
    Rotation3 rotation = {0, PI / 2, 0};

    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);

    float fov = -1.0f;
    float aspectRatio = -1.0f;
    float zNear = -1.0f;
    float zFar = -1.0f;

    bool isInitialized = false;
};