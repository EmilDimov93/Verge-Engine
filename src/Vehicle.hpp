// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Mesh.hpp"
#include "Input.hpp"
#include "definitions.hpp"

struct VE_STRUCT_VEHICLE_CREATE_INFO
{
    Mesh *pBody = nullptr;
    Mesh *pTire = nullptr;

    uint32_t horsePower = 100;
    float weight = 1200.f;
    uint32_t gearCount = 6;
    uint32_t maxRpm = 6000;
    float brakingForce = 1.0f;
    bool isAutomatic = true;

    VEKey accelerateKey = VE_KEY_UNKNOWN;
    VEKey brakeKey = VE_KEY_UNKNOWN;
    VEKey turnLeftKey = VE_KEY_UNKNOWN;
    VEKey turnRightKey = VE_KEY_UNKNOWN;

    float *pGearRatios = nullptr;
    float finalDriveRatio = 3.42f;
    float drivetrainEfficiency = 0.85f;
    float wheelRadius = 0.3f;
    float dragAccel = 0.5f;
    float dragCoeff = 0.31f;
    float frontalArea = 2.3f;

    float idleRpm = 800.f;
};

class Vehicle
{
public:
    void init(const VE_STRUCT_VEHICLE_CREATE_INFO &info);

    void update(ve_time deltaTime);

private:
    void accelerate(ve_time deltaTime);
    void idle(ve_time deltaTime);
    void brake(ve_time deltaTime);
    void turnLeft();
    void turnRight();

    Mesh body;
    Mesh tire;

    uint32_t horsePower;
    float weight;
    uint32_t gearCount;
    uint32_t maxRpm;
    bool isAutomatic;
    float brakingForce;

    VEKey accelerateKey;
    VEKey brakeKey;
    VEKey turnLeftKey;
    VEKey turnRightKey;

    std::vector<float> gearRatios;
    float finalDriveRatio;
    float drivetrainEfficiency;
    float wheelRadius;
    float idleRpm;
    float dragAccel;
    float dragCoeff;
    float frontalArea;

    // Runtime
    Position3 position;
    Rotation3 rotation;
    float tireRotation;
    float speed;
    uint32_t gear;
    float rpm;
};