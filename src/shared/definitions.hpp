// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <limits>

#include <vector>

namespace VE
{

    constexpr float FLOAT_MIN = std::numeric_limits<float>::lowest();

    constexpr float PI = glm::radians(180.0f);

    template <typename Tag>
    class Handle
    {
    public:
        constexpr Handle() = default;
        constexpr explicit Handle(uint64_t value) : value(value) {}

        constexpr bool isValid() const { return value != INVALID; }
        constexpr uint64_t getValue() const { return value; }

        friend bool operator==(const Handle &, const Handle &) = default;

    private:
        static constexpr uint64_t INVALID = 0;
        uint64_t value = INVALID;
    };

    struct ModelTag
    {
    };
    struct ModelInstanceTag
    {
    };
    struct PlayerTag
    {
    };
    struct VehicleTag
    {
    };
    struct PropTag
    {
    };
    struct TriggerTag
    {
    };

    using ModelHandle = Handle<ModelTag>;
    using ModelInstanceHandle = Handle<ModelInstanceTag>;
    using PlayerHandle = Handle<PlayerTag>;
    using VehicleHandle = Handle<VehicleTag>;
    using PropHandle = Handle<PropTag>;
    using TriggerHandle = Handle<TriggerTag>;

    constexpr ModelHandle INVALID_MODEL_HANDLE{0};

    using milliseconds_t = double;
    using color_t = glm::vec4;

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

        glm::mat4 toMat() const
        {
            glm::mat4 mat(1.0f);

            mat = glm::translate(mat, glm::vec3(static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z)));

            mat = glm::rotate(mat, static_cast<float>(rotation.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
            mat = glm::rotate(mat, static_cast<float>(rotation.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            mat = glm::rotate(mat, static_cast<float>(rotation.roll), glm::vec3(0.0f, 0.0f, 1.0f));

            mat = glm::scale(mat, glm::vec3(static_cast<float>(scale.x), static_cast<float>(scale.y), static_cast<float>(scale.z)));

            return mat;
        }
    };

    template <typename T>
    inline T clamp(T v, T lo, T hi)
    {
        return (v < lo) ? lo : (v > hi) ? hi
                                        : v;
    }

    template <typename T>
    inline T clamp01(T v)
    {
        return (v < T(0)) ? T(0) : (v > T(1)) ? T(1)
                                              : v;
    }

    constexpr float AvoidZero(float x)
    {
        return (x == 0.0f) ? FLT_TRUE_MIN : x;
    }

    struct VehicleInputState
    {
        float throttle = 0.0f;
        float brake = 0.0f;
        float handbrake = 0.0f;
        float clutch = 0.0f;

        float steer = 0.0f;

        bool shiftUp = false;
        bool shiftDown = false;

        bool starter = false;

        float moveCameraLeft = 0.0f;
        float moveCameraRight = 0.0f;
        float moveCameraUp = 0.0f;
        float moveCameraDown = 0.0f;
    };

    inline float wrapRadToPi(float angleRad)
    {
        angleRad = std::fmod(angleRad + PI, 2.0f * PI);
        if (angleRad < 0.0f)
            angleRad += 2.0f * PI;
        return angleRad - PI;
    }

}