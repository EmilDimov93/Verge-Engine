// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

class Controller
{
public:
    virtual VehicleInputState getVehicleInputState() const = 0;
    virtual VehicleHandle getVehicleHandle() const = 0;
    virtual ~Controller() = default;
};