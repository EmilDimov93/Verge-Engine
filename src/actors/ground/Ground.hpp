// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../Log.hpp"

#include "../../definitions.hpp"

#include <vector>

struct Surface
{
    float friction;
    glm::vec3 color;
    // Texture texture;
};

struct Ground
{
    uint32_t w = 0, h = 0;

    std::vector<float> heightMap;
    std::vector<uint8_t> surfaceMap;

    void resize(uint32_t w, uint32_t h)
    {
        this->w = w;
        this->h = h;

        heightMap.resize(w * h);
        surfaceMap.resize(w * h);
    }

    size_t getHeightAt(uint32_t x, uint32_t y) const
    {
        if(x > w || y > h){
            Log::add('A', 191);
            return 0;
        }
        return heightMap[size_t(y) * w + x];
    }
    size_t getSurfaceAt(uint32_t x, uint32_t y) const
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