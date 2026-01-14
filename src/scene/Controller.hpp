// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <cstdint>

struct VehicleInputState{
    float throttle = 0;
    float brake = 0;

    float steer = 0;

    bool shiftUp = false;
    bool shiftDown = false;
};

class Controller{
public:
    virtual const VehicleInputState getVehicleInputState() = 0;
    virtual const uint64_t getVehicleIndex() = 0;
    virtual ~Controller() = default;
};