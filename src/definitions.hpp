// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <iostream> // temporary
#include <fstream>

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

struct Size2 {
    uint32_t w, h;

    constexpr Size2(uint32_t width = 0, uint32_t height = 0) : w(width), h(height) {}
};

struct Size3 {
    uint32_t w, h, d;

    constexpr Size3(uint32_t width = 0, uint32_t height = 0, uint32_t depth = 0) : w(width), h(height), d(depth) {}
};

struct Position2 {
    double x, y;

    constexpr Position2(double x_val = 0.0, double y_val = 0.0) : x(x_val), y(y_val) {}
};

struct Position3 {
    double x, y, z;

    constexpr Position3(double x_val = 0.0, double y_val = 0.0, double z_val = 0.0) : x(x_val), y(y_val), z(z_val) {}
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 col;

    Vertex(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& color = glm::vec3(1.0f)) : pos(position), col(color) {}
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