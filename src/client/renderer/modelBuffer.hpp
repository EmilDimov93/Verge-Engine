// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "meshBuffer.hpp"

#include "../../shared/drawData.hpp"
#include "../../shared/definitions.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace VE
{
    struct ModelBuffer
    {
        ModelHandle handle;

        std::vector<MeshBuffer> meshBuffers;
        std::vector<Material> materials;

        uint64_t version = 0;

        ModelBuffer(ModelHandle handle) : handle(handle) {}
    };
}