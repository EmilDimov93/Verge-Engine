// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

namespace VE
{

    inline float KelvinToCelsius(float kelvin)
    {
        return kelvin - 273.15f;
    }

    struct Environment
    {
        color_t backgroundColor = color_t(1.0f);
        float airDensityKgpm3 = 1.225f;
        float gravityMps2 = 9.81f;
        float temperatureK = 293.15f;
    };

}