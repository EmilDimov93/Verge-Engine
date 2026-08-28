// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/log.hpp"

#include "../../shared/definitions.hpp"

namespace VE
{

using SurfaceTypeIndex = uint32_t;

struct SurfaceTypeCreateInfo
{
    float friction = 1.f;
    color_t color{0};
    float heightDistortion = 0.f;
};

struct SurfaceType
{
    float friction;
    color_t color;
    float heightDistortion;
};

class Surface
{
public:
    glm::uvec2 size = {0, 0};

    float tileSize = 1.f;

    std::vector<float> heightMap;
    std::vector<SurfaceTypeIndex> surfaceTypeMap;

    glm::vec3 position;

    void resize(glm::uvec2 size)
    {
        this->size.x = size.x;
        this->size.y = size.y;

        heightMap.resize(size.x * size.y);
        surfaceTypeMap.resize(size.x * size.y);
    }

    [[nodiscard]] float sampleHeight(const glm::vec3 &pos) const // world coordinates
    {
        if (pos.x < position.x - size.x * tileSize / 2 || pos.x > position.x + size.x * tileSize / 2 || pos.z < position.z - size.y * tileSize / 2 || pos.z > position.z + size.y * tileSize / 2)
        {
            return FLOAT_MIN;
        }

        float localX = size.x / 2.f + (pos.x - position.x) / tileSize;
        float localZ = size.y / 2.f + (pos.z - position.z) / tileSize;

        int localXLower = std::floor(localX);
        int localXUpper = std::ceil(localX);

        int localZLower = std::floor(localZ);
        int localZUpper = std::ceil(localZ);

        float avg = (getHeightAt(localXLower, localZLower) + getHeightAt(localXLower, localZUpper) + getHeightAt(localXUpper, localZLower) + getHeightAt(localXUpper, localZUpper)) / 4;

        return avg;
    }

    [[nodiscard]] SurfaceTypeIndex sampleSurfaceTypeIndex(const glm::vec3 &pos) const // world coordinates
    {
        if (pos.x < position.x - size.x * tileSize / 2 || pos.x > position.x + size.x * tileSize / 2 || pos.z < position.z - size.y * tileSize / 2 || pos.z > position.z + size.y * tileSize / 2)
        {
            return 0;
        }

        return getSurfaceTypeAt(size.x / 2.f + (pos.x - position.x) / tileSize, size.y / 2.f + (pos.z - position.z) / tileSize);
    }

private:
    [[nodiscard]] float getHeightAt(uint32_t x, uint32_t y) const // grid coordinates
    {
        if (x >= size.x || y >= size.y)
            return FLOAT_MIN;

        return heightMap[size_t(y) * size.x + x];
    }

    [[nodiscard]] SurfaceTypeIndex getSurfaceTypeAt(uint32_t x, uint32_t y) const // grid coordinates
    {
        if (x >= size.x || y >= size.y)
            return 0;

        return surfaceTypeMap[size_t(y) * size.x + x];
    }
};

}