// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "imageAttachment.hpp"

namespace VE
{
    struct GeometryBuffer
    {
        ImageAttachment depth;
        ImageAttachment normal;
        ImageAttachment metallic;
        ImageAttachment roughness;
    };
}