// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <functional>
#include <vector>

namespace VE
{
    constexpr float FLOAT_MIN = std::numeric_limits<float>::lowest();

    constexpr float PI = glm::radians(180.f);

    struct ModelTag
    {
    };
    struct ModelInstanceTag
    {
    };
    struct WidgetTag
    {
    };
    struct WidgetInstanceTag
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

    template <typename Tag>
    class Handle
    {
    public:
        constexpr Handle() = default;
        constexpr explicit Handle(uint64_t value) : value(value) {}

        [[nodiscard]] constexpr bool isValid() const { return value != INVALID_VALUE; }
        [[nodiscard]] constexpr uint64_t getValue() const { return value; }

        [[nodiscard]] friend bool operator==(const Handle &, const Handle &) = default;

        static const Handle INVALID;

        friend std::ostream &operator<<(std::ostream &outputStream, const Handle &handle)
        {
            std::string prefix = "U";
            if constexpr (std::is_same_v<Tag, ModelTag>)
                prefix = "MO";
            else if constexpr (std::is_same_v<Tag, ModelInstanceTag>)
                prefix = "MI";
            else if constexpr (std::is_same_v<Tag, WidgetTag>)
                prefix = "WG";
            else if constexpr (std::is_same_v<Tag, WidgetInstanceTag>)
                prefix = "WI";
            else if constexpr (std::is_same_v<Tag, PlayerTag>)
                prefix = "PL";
            else if constexpr (std::is_same_v<Tag, VehicleTag>)
                prefix = "VH";
            else if constexpr (std::is_same_v<Tag, PropTag>)
                prefix = "PR";
            else if constexpr (std::is_same_v<Tag, TriggerTag>)
                prefix = "TR";

            // Base 26: A-Z
            constexpr uint32_t width = 14;
            std::string result(width, 'A');
            uint64_t value = handle.getValue();
            for (uint32_t i = width; i > 0 && value > 0; --i)
            {
                result[i - 1] = char('A' + value % 26);
                value /= 26;
            }

            outputStream << prefix << result;
            return outputStream;
        }

    private:
        static constexpr uint64_t INVALID_VALUE = 0;
        uint64_t value = INVALID_VALUE;
    };

    template <typename Tag>
    inline constexpr Handle<Tag> Handle<Tag>::INVALID{};

    using ModelHandle = Handle<ModelTag>;
    using ModelInstanceHandle = Handle<ModelInstanceTag>;
    using WidgetHandle = Handle<WidgetTag>;
    using WidgetInstanceHandle = Handle<WidgetInstanceTag>;
    using PlayerHandle = Handle<PlayerTag>;
    using VehicleHandle = Handle<VehicleTag>;
    using PropHandle = Handle<PropTag>;
    using TriggerHandle = Handle<TriggerTag>;

    using seconds_t = double;
    using color_t = glm::vec4;

    struct Transform
    {
        glm::vec3 position{};
        glm::vec3 rotation{};
        glm::vec3 scale = glm::vec3(1.f);

        constexpr Transform(glm::vec3 p = {}, glm::vec3 r = {}, glm::vec3 s = glm::vec3(1.f)) : position(p), rotation(r), scale(s) {}

        [[nodiscard]] glm::mat4 toMat() const
        {
            glm::mat4 mat(1.f);

            mat = glm::translate(mat, position);

            mat = glm::rotate(mat, static_cast<float>(rotation.x), glm::vec3(1.f, 0.f, 0.f));
            mat = glm::rotate(mat, static_cast<float>(rotation.y), glm::vec3(0.f, 1.f, 0.f));
            mat = glm::rotate(mat, static_cast<float>(rotation.z), glm::vec3(0.f, 0.f, 1.f));

            mat = glm::scale(mat, glm::vec3(static_cast<float>(scale.x), static_cast<float>(scale.y), static_cast<float>(scale.z)));

            return mat;
        }
    };

    template <typename T>
    [[nodiscard]] inline T clamp(T v, T lo, T hi)
    {
        return (v < lo) ? lo : (v > hi) ? hi
                                        : v;
    }

    template <typename T>
    [[nodiscard]] inline T clamp01(T v)
    {
        return (v < T(0)) ? T(0) : (v > T(1)) ? T(1)
                                              : v;
    }

    [[nodiscard]] constexpr float AvoidZero(float x)
    {
        return (x == 0.f) ? FLT_TRUE_MIN : x;
    }

    struct VehicleInputState
    {
        float throttle = 0.f;
        float brake = 0.f;
        float handbrake = 0.f;
        float clutch = 0.f;

        float steer = 0.f;

        bool shiftUp = false;
        bool shiftDown = false;

        bool starter = false;

        float moveCameraLeft = 0.f;
        float moveCameraRight = 0.f;
        float moveCameraUp = 0.f;
        float moveCameraDown = 0.f;
    };

    [[nodiscard]] inline float wrapRadToPi(float angleRad)
    {
        angleRad = std::fmod(angleRad + PI, 2.f * PI);
        if (angleRad < 0.f)
            angleRad += 2.f * PI;
        return angleRad - PI;
    }

    inline color_t srgbToLinear(color_t srgb)
    {
        constexpr float SRGB_GAMMA = 2.2f;
        return color_t(glm::pow(glm::vec3(srgb), glm::vec3(SRGB_GAMMA)), srgb.a);
    }
}

namespace std
{
    template <typename Tag>
    struct hash<VE::Handle<Tag>>
    {
        [[nodiscard]] size_t operator()(const VE::Handle<Tag> &handle) const noexcept
        {
            return hash<uint64_t>{}(handle.getValue());
        }
    };
}