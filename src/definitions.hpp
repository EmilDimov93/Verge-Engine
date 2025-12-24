// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.1415926f

using ve_time = double;

struct Size2 {
    uint32_t w, h;

    constexpr Size2(uint32_t width = 0, uint32_t height = 0) : w(width), h(height) {}

    operator glm::vec2() const { return {static_cast<float>(w), static_cast<float>(h)}; }
};

struct Size3 {
    uint32_t w, h, d;

    constexpr Size3(uint32_t width = 0, uint32_t height = 0, uint32_t depth = 0) : w(width), h(height), d(depth) {}

    operator glm::vec3() const { return {static_cast<float>(w), static_cast<float>(h), static_cast<float>(d)}; }
};

struct Position2 {
    double x, y;

    constexpr Position2(double x_val = 0.0, double y_val = 0.0) : x(x_val), y(y_val) {}

    explicit operator glm::dvec2() const { return {x, y}; }
};

struct Position3 {
    double x, y, z;

    constexpr Position3(double x_val = 0.0, double y_val = 0.0, double z_val = 0.0) : x(x_val), y(y_val), z(z_val) {}

    explicit operator glm::dvec3() const { return {x, y, z}; }
};

struct Rotation3 {
    double pitch, yaw, roll;

    constexpr Rotation3(double pitch_val = 0.0, double yaw_val = 0.0, double roll_val = 0.0) : pitch(pitch_val), yaw(yaw_val), roll(roll_val) {}

    explicit operator glm::dvec3() const { return {pitch, yaw, roll}; }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 col;

    Vertex(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& color = glm::vec3(1.0f)) : pos(position), col(color) {}
};