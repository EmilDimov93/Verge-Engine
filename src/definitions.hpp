// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <iostream> // temporary
#include <fstream>

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

struct Size2 { 
    uint32_t w;
    uint32_t h; 
};

struct Size3 { 
    uint32_t w;
    uint32_t h;
    uint32_t d; 
};

struct Position2 { 
    double x;
    double y;
};

struct Position3 { 
    double x;
    double y;
    double z;
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 col;
};

static std::vector<char> readFile(const std::string &fileName){
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);

    if(!file.is_open()){
        return {};
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);

    file.read(fileBuffer.data(), fileSize);

    file.close();

    return fileBuffer;
}