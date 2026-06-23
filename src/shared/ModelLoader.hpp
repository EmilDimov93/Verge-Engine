// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "DrawData.hpp"

#include <vector>

namespace VE
{
    [[nodiscard]] std::pair<std::vector<Mesh>, std::vector<Material>> loadModel(const std::string &filePath);
}