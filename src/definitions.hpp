// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.14159265358979323846264338327950288419716939937510f

using ve_time = double;

struct Size2
{
    uint32_t w, h;

    constexpr Size2(uint32_t width = 0, uint32_t height = 0) : w(width), h(height) {}
};

struct Size3
{
    uint32_t w, h, d;

    constexpr Size3(uint32_t width = 0, uint32_t height = 0, uint32_t depth = 0) : w(width), h(height), d(depth) {}
};

struct Position2
{
    double x, y;

    constexpr Position2(double x_val = 0.0, double y_val = 0.0) : x(x_val), y(y_val) {}
};

struct Position3
{
    double x, y, z;

    constexpr Position3(double x_val = 0.0, double y_val = 0.0, double z_val = 0.0) : x(x_val), y(y_val), z(z_val) {}
};

struct Rotation3
{
    double pitch, yaw, roll;

    constexpr Rotation3(double pitch_val = 0.0, double yaw_val = 0.0, double roll_val = 0.0) : pitch(pitch_val), yaw(yaw_val), roll(roll_val) {}
};

struct Scale3
{
    double x, y, z;

    constexpr Scale3(double x_val = 1.0, double y_val = 1.0, double z_val = 1.0) : x(x_val), y(y_val), z(z_val) {}
};

struct Transform
{
    Position3 position{};
    Rotation3 rotation{};
    Scale3 scale{};

    constexpr Transform(Position3 p = {}, Rotation3 r = {}, Scale3 s = {}) : position(p), rotation(r), scale(s) {}
};

inline glm::mat4 transformToMat(const Transform& transform)
{
    glm::mat4 mat(1.0f);

    mat = glm::translate(mat, glm::vec3(static_cast<float>(transform.position.x), static_cast<float>(transform.position.y), static_cast<float>(transform.position.z)));

    mat = glm::rotate(mat, static_cast<float>(transform.rotation.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::rotate(mat, static_cast<float>(transform.rotation.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::rotate(mat, static_cast<float>(transform.rotation.roll), glm::vec3(0.0f, 0.0f, 1.0f));

    mat = glm::scale(mat, glm::vec3(static_cast<float>(transform.scale.x), static_cast<float>(transform.scale.y), static_cast<float>(transform.scale.z)));

    return mat;
}

template <typename T>
inline T clamp(T v, T lo, T hi)
{
    return (v < lo) ? lo : (v > hi) ? hi : v;
}