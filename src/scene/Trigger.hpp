// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../definitions.hpp"

class Trigger
{
public:
    Position3 position;
    uint32_t meshIndex;

    bool isTriggered();
};