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
    virtual VehicleInputState getVehicleInputState() = 0; // Should be const
    virtual uint64_t getVehicleIndex() const = 0;
    virtual ~Controller() = default;
};