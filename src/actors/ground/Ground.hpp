// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../Log.hpp"

#include "../../definitions.hpp"

#include <vector>

struct VE_STRUCT_SURFACE_CREATE_INFO
{
    float friction = 1.0f;
    glm::vec3 color = {0, 0, 0};
    glm::vec3 colorDistortion = {0, 0, 0};
    float heightDistortion = 0;
};

struct Surface
{
    float friction;
    glm::vec3 color;
    glm::vec3 colorDistortion;
    float heightDistortion;
    // Texture texture;
};

struct Ground
{
    uint32_t w = 0, h = 0;

    std::vector<float> heightMap;
    std::vector<uint8_t> surfaceMap;

    Transform transform;

    void resize(uint32_t w, uint32_t h)
    {
        this->w = w;
        this->h = h;

        heightMap.resize(w * h);
        surfaceMap.resize(w * h);
    }

    float sampleHeight(float x, float z) const // world coordinates
    {
        if(x < transform.position.x - w / 2 || x > transform.position.x + w / 2 || z < transform.position.z - h / 2 || z > transform.position.z + h / 2){
            Log::add('A', 191);
            return 0;
        }

        float localX = w / 2 + x - transform.position.x;
        float localZ = h / 2 + z - transform.position.z;

        int localXLower = (int)localX;
        int localXUpper = localXLower + 1;

        int localZLower = (int)localZ;
        int localZUpper = localZLower + 1;

        float avg = getHeightAt(localXLower, localZLower) + getHeightAt(localXLower, localZUpper) + getHeightAt(localXUpper, localZLower) + getHeightAt(localXUpper, localZUpper);
        avg = avg / 4;

        return avg;
    }

    float getHeightAt(uint32_t x, uint32_t y) const // grid coordinates
    {
        if(x > w || y > h){
            Log::add('A', 191);
            return 0;
        }
        return heightMap[size_t(y) * w + x];
    }
    uint32_t getSurfaceAt(uint32_t x, uint32_t y) const // grid coordinates
    {
        if(x > w || y > h){
            Log::add('A', 191);
            return 0;
        }
        return surfaceMap[size_t(y) * w + x];
    }

    void setHeightAt(uint32_t x, uint32_t y, float value)
    {
        if(x > w || y > h){
            Log::add('A', 191);
            return;
        }
        heightMap[size_t(y) * w + x] = value;
    }
    void setSurfaceAt(uint32_t x, uint32_t y, uint8_t surfaceIndex)
    {
        if(x > w || y > h){
            Log::add('A', 191);
            return;
        }
        surfaceMap[size_t(y) * w + x] = surfaceIndex;
    }
};