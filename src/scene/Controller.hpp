// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

struct VehicleInputState{
    float throttle = 0;
    float brake = 0;
    bool handbrake = false;

    float steer = 0;

    bool shiftUp = false;
    bool shiftDown = false;
};

class Controller{
public:
    virtual VehicleInputState getVehicleInputState() const = 0;
    virtual VehicleHandle getVehicleHandle() const = 0;
    virtual ~Controller() = default;
};