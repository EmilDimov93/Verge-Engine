// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Mesh.hpp"

class Vehicle
{
public:
    //Vehicle(Mesh &&bodyMesh, Mesh &&tireMesh) : body(std::move(bodyMesh)), tire(std::move(tireMesh)){}

//private:
    Mesh body;
    Mesh tire;
    float weight;
    float brakingForce;
    uint32_t gearCount;
    uint32_t horsePower;
    uint32_t maxRpm;

    Position3 position;
    Rotation3 rotation;
    float tireRotation;
    float speed;
    uint32_t gear;
    float rpm;

    bool isGasDown;

    void updateRmp();
};