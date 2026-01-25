// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/Log.hpp"

#include "../../shared/definitions.hpp"

struct VE_STRUCT_SURFACE_TYPE_CREATE_INFO
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
    uint32_t w = 0, h = 0;

    std::vector<float> heightMap;
    std::vector<uint8_t> surfaceTypeMap;

    // Rotation and scale not implemented
    Transform transform;

    void resize(Size2 size)
    {
        this->w = size.w;
        this->h = size.h;

        heightMap.resize(w * h);
        surfaceTypeMap.resize(w * h);
    }

    float sampleHeight(float x, float z) const // world coordinates
    {
        if (x < transform.position.x - w / 2 || x > transform.position.x + w / 2 || z < transform.position.z - h / 2 || z > transform.position.z + h / 2)
        {
            Log::add('A', 191);
            return 0;
        }

        float localX = w / 2 + x - transform.position.x;
        float localZ = h / 2 + z - transform.position.z;

        int localXLower = (int)localX;
        int localXUpper = localXLower + 1;

        int localZLower = (int)localZ;
        int localZUpper = localZLower + 1;

        float avg = (getHeightAt(localXLower, localZLower) + getHeightAt(localXLower, localZUpper) + getHeightAt(localXUpper, localZLower) + getHeightAt(localXUpper, localZUpper)) / 4;

        return avg;
    }

    float sampleSurfaceTypeIndex(float x, float z) const
    {
        if (x < transform.position.x - w / 2 || x > transform.position.x + w / 2 || z < transform.position.z - h / 2 || z > transform.position.z + h / 2)
        {
            Log::add('A', 191);
            return 0;
        }

        return getSurfaceTypeAt(w / 2 + x - transform.position.x, h / 2 + z - transform.position.z);
    }

    float getHeightAt(uint32_t x, uint32_t y) const // grid coordinates
    {
        if (x >= w || y >= h || x < 0 || y < 0)
        {
            Log::add('A', 191);
            return 0;
        }
        return heightMap[size_t(y) * w + x];
    }
    uint32_t getSurfaceTypeAt(uint32_t x, uint32_t y) const // grid coordinates
    {
        if (x >= w || y >= h || x < 0 || y < 0)
        {
            Log::add('A', 191);
            return 0;
        }
        return surfaceTypeMap[size_t(y) * w + x];
    }

    void setHeightAt(uint32_t x, uint32_t y, float value)
    {
        if (x > w - 1 || y > h - 1)
        {
            Log::add('A', 191);
            return;
        }
        heightMap[size_t(y) * w + x] = value;
    }
    void setSurfaceTypeAt(uint32_t x, uint32_t y, uint8_t surfaceIndex)
    {
        if (x > w || y > h)
        {
            Log::add('A', 191);
            return;
        }
        surfaceTypeMap[size_t(y) * w + x] = surfaceIndex;
    }

private:
};