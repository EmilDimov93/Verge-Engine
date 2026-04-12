// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/Log.hpp"

#include "../../shared/definitions.hpp"

namespace VE
{

using SurfaceTypeIndex = uint32_t;

struct SurfaceTypeCreateInfo
{
    float friction = 1.0f;
    glm::vec3 color = {0, 0, 0};
    glm::vec3 colorDistortion = {0, 0, 0};
    float heightDistortion = 0;
};

struct SurfaceType
{
    float friction;
    glm::vec3 color;
    glm::vec3 colorDistortion;
    float heightDistortion;
    // Texture texture;
};

class Surface
{
public:
    Size2 size = {0, 0};

    float tileSize = 1.0f;

    std::vector<float> heightMap;
    std::vector<SurfaceTypeIndex> surfaceTypeMap;

    Position3 position;

    void resize(Size2 size)
    {
        this->size.w = size.w;
        this->size.h = size.h;

        heightMap.resize(size.w * size.h);
        surfaceTypeMap.resize(size.w * size.h);
    }

    float sampleHeight(const Position3 &pos) const // world coordinates
    {
        if (pos.x < position.x - size.w * tileSize / 2 || pos.x > position.x + size.w * tileSize / 2 || pos.z < position.z - size.h * tileSize / 2 || pos.z > position.z + size.h * tileSize / 2)
        {
            return FLOAT_MIN;
        }

        float localX = size.w / 2.0f + (pos.x - position.x) / tileSize;
        float localZ = size.h / 2.0f + (pos.z - position.z) / tileSize;

        int localXLower = std::floor(localX);
        int localXUpper = std::ceil(localX);

        int localZLower = std::floor(localZ);
        int localZUpper = std::ceil(localZ);

        float avg = (getHeightAt(localXLower, localZLower) + getHeightAt(localXLower, localZUpper) + getHeightAt(localXUpper, localZLower) + getHeightAt(localXUpper, localZUpper)) / 4;

        return avg;
    }

    SurfaceTypeIndex sampleSurfaceTypeIndex(const Position3 &pos) const // world coordinates
    {
        if (pos.x < position.x - size.w * tileSize / 2 || pos.x > position.x + size.w * tileSize / 2 || pos.z < position.z - size.h * tileSize / 2 || pos.z > position.z + size.h * tileSize / 2)
        {
            return 0;
        }

        return getSurfaceTypeAt(size.w / 2.0f + (pos.x - position.x) / tileSize, size.h / 2.0f + (pos.z - position.z) / tileSize);
    }

private:
    float getHeightAt(uint32_t x, uint32_t y) const // grid coordinates
    {
        if (x >= size.w || y >= size.h)
            return FLOAT_MIN;

        return heightMap[size_t(y) * size.w + x];
    }

    SurfaceTypeIndex getSurfaceTypeAt(uint32_t x, uint32_t y) const // grid coordinates
    {
        if (x >= size.w || y >= size.h)
            return 0;

        return surfaceTypeMap[size_t(y) * size.w + x];
    }
};

}