// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

namespace VE
{

    class Controller
    {
    public:
        [[nodiscard]] virtual VehicleInputState getVehicleInputState() const = 0;
        [[nodiscard]] virtual VehicleHandle getVehicleHandle() const = 0;
        virtual ~Controller() = default;
    };

}