// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Mesh.hpp"

class Vehicle
{
public:
    Vehicle(Mesh &&bodyMesh, Mesh &&tireMesh) : body(std::move(bodyMesh)), tire(std::move(tireMesh)){}

private:
    Mesh body;
    Mesh tire;

    float tireRotation;
    float speed;
    float weight;
};