// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Mesh.hpp"
#include "Input.hpp"

class Vehicle
{
public:
    // Vehicle(Mesh &&bodyMesh, Mesh &&tireMesh) : body(std::move(bodyMesh)), tire(std::move(tireMesh)){}

    // private:
    Mesh body;
    Mesh tire;
    float weight;
    float brakingForce;
    uint32_t gearCount;
    uint32_t horsePower;
    uint32_t maxRpm;
    bool isAutomatic;

    VEKey accelerateKey;

    Position3 position;
    Rotation3 rotation;
    float tireRotation;
    float speed;
    uint32_t gear;
    float rpm;

    const float gearRatios[8] = {5.519f, 3.184f, 2.050f, 1.492f, 1.235f, 1.000f, 0.801f, 0.673f};
    const float finalDriveRatio = 3.2f;
    const float drivetrainEfficiency = 0.9f;
    const float wheelRadius = 0.31f;
    const float idleRpm = 800.0f;
    const float dragAccel = 0.5f;
    const float dragCoeff = 0.31f;
    const float frontalArea = 2.3f;

    void update(double deltaTime);
private:
    void accelerate(double deltaTime);
};