// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "imageAttachment.hpp"

#include "../../../shared/definitions.hpp"

namespace VE
{
    struct ShadowMap
    {
        ModelInstanceHandle instanceHandle;
        ImageAttachment depthAttachment;
    };
}