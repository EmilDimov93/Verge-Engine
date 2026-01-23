// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

struct Environment
{
    ve_color_t backgroundColor;
    float airDensityKgpm3 = 1.225f;
    float gravityMps2 = 9.81f;
    float temperatureK = 293.15f;
};