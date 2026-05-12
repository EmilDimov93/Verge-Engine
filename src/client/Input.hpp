// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/definitions.hpp"

#include <GLFW/glfw3.h>

namespace VE
{

    enum Key
    {
        KEY_UNKNOWN = -1,

        KEY_A = GLFW_KEY_A,
        KEY_B = GLFW_KEY_B,
        KEY_C = GLFW_KEY_C,
        KEY_D = GLFW_KEY_D,
        KEY_E = GLFW_KEY_E,
        KEY_F = GLFW_KEY_F,
        KEY_G = GLFW_KEY_G,
        KEY_H = GLFW_KEY_H,
        KEY_I = GLFW_KEY_I,
        KEY_J = GLFW_KEY_J,
        KEY_K = GLFW_KEY_K,
        KEY_L = GLFW_KEY_L,
        KEY_M = GLFW_KEY_M,
        KEY_N = GLFW_KEY_N,
        KEY_O = GLFW_KEY_O,
        KEY_P = GLFW_KEY_P,
        KEY_Q = GLFW_KEY_Q,
        KEY_R = GLFW_KEY_R,
        KEY_S = GLFW_KEY_S,
        KEY_T = GLFW_KEY_T,
        KEY_U = GLFW_KEY_U,
        KEY_V = GLFW_KEY_V,
        KEY_W = GLFW_KEY_W,
        KEY_X = GLFW_KEY_X,
        KEY_Y = GLFW_KEY_Y,
        KEY_Z = GLFW_KEY_Z,
        KEY_0 = GLFW_KEY_0,
        KEY_1 = GLFW_KEY_1,
        KEY_2 = GLFW_KEY_2,
        KEY_3 = GLFW_KEY_3,
        KEY_4 = GLFW_KEY_4,
        KEY_5 = GLFW_KEY_5,
        KEY_6 = GLFW_KEY_6,
        KEY_7 = GLFW_KEY_7,
        KEY_8 = GLFW_KEY_8,
        KEY_9 = GLFW_KEY_9,
        KEY_F1 = GLFW_KEY_F1,
        KEY_F2 = GLFW_KEY_F2,
        KEY_F3 = GLFW_KEY_F3,
        KEY_F4 = GLFW_KEY_F4,
        KEY_F5 = GLFW_KEY_F5,
        KEY_F6 = GLFW_KEY_F6,
        KEY_F7 = GLFW_KEY_F7,
        KEY_F8 = GLFW_KEY_F8,
        KEY_F9 = GLFW_KEY_F9,
        KEY_F10 = GLFW_KEY_F10,
        KEY_F11 = GLFW_KEY_F11,
        KEY_F12 = GLFW_KEY_F12,
        KEY_SPACE = GLFW_KEY_SPACE,
        KEY_ENTER = GLFW_KEY_ENTER,
        KEY_ESCAPE = GLFW_KEY_ESCAPE,
        KEY_TAB = GLFW_KEY_TAB,
        KEY_BACKSPACE = GLFW_KEY_BACKSPACE,
        KEY_INSERT = GLFW_KEY_INSERT,
        KEY_DELETE = GLFW_KEY_DELETE,
        KEY_RIGHT = GLFW_KEY_RIGHT,
        KEY_LEFT = GLFW_KEY_LEFT,
        KEY_UP = GLFW_KEY_UP,
        KEY_DOWN = GLFW_KEY_DOWN,
        KEY_HOME = GLFW_KEY_HOME,
        KEY_END = GLFW_KEY_END,
        KEY_PAGE_UP = GLFW_KEY_PAGE_UP,
        KEY_PAGE_DOWN = GLFW_KEY_PAGE_DOWN,
        KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
        KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
        KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
        KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
        KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT,
        KEY_RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
        KEY_RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER,
        KEY_MENU = GLFW_KEY_MENU,

        KEY_COUNT
    };

    enum MouseBtn
    {
        MOUSE_BTN_UNKNOWN = -1,

        MOUSE_BTN_LEFT = GLFW_MOUSE_BUTTON_1,
        MOUSE_BTN_RIGHT = GLFW_MOUSE_BUTTON_2,
        MOUSE_BTN_MIDDLE = GLFW_MOUSE_BUTTON_3,
        MOUSE_BTN_4 = GLFW_MOUSE_BUTTON_4,
        MOUSE_BTN_5 = GLFW_MOUSE_BUTTON_5,
        MOUSE_BTN_6 = GLFW_MOUSE_BUTTON_6,
        MOUSE_BTN_7 = GLFW_MOUSE_BUTTON_7,
        MOUSE_BTN_8 = GLFW_MOUSE_BUTTON_8,

        MOUSE_BTN_COUNT
    };

    enum AxisMapping
    {
        AXIS_MAPPING_FULL,
        AXIS_MAPPING_FULL_INVERTED,
        AXIS_MAPPING_POSITIVE_HALF,
        AXIS_MAPPING_NEGATIVE_HALF
    };

    using ControllerBtn = uint32_t;

    struct ControllerAxis
    {
        uint32_t index;
        AxisMapping mapping;
    };

    constexpr ControllerBtn CONTROLLER_BTN_UNKNOWN = UINT32_MAX;
    constexpr ControllerAxis CONTROLLER_AXIS_UNKNOWN = {UINT32_MAX, AXIS_MAPPING_FULL};

    class Input
    {
    public:
        static void init(GLFWwindow *windowRef);

        static void refresh();

        // Keyboard
        static bool isDown(Key key);
        static bool isUp(Key key);
        static bool isPressed(Key key);
        static bool isReleased(Key key);

        // Mouse
        static bool isDown(MouseBtn btn);
        static bool isUp(MouseBtn btn);
        static bool isPressed(MouseBtn btn);
        static bool isReleased(MouseBtn btn);

        static Position2 getMousePos();

        // Controller
        static void setController(uint8_t id);
        static void setAxisDeadZone(float deadZone);

        static bool isDown(ControllerBtn btn);
        static bool isUp(ControllerBtn btn);
        static bool isPressed(ControllerBtn btn);
        static bool isReleased(ControllerBtn btn);

        static float getAxis(ControllerAxis axis);

        enum KeyState
        {
            KEY_STATE_DOWN,
            KEY_STATE_UP,
            KEY_STATE_PRESSED,
            KEY_STATE_RELEASED
        };

    private:
        static KeyState keyStates[KEY_COUNT];
        static KeyState mouseBtnStates[MOUSE_BTN_COUNT];

        static Position2 mousePosition;

        static int controllerId;
        static bool controllerConnected;

        static std::vector<KeyState> controllerBtnStates;
        static std::vector<float> controllerAxes;

        static float axisDeadZone;

        static GLFWwindow *window;
    };

    enum KeyType
    {
        KEY_TYPE_KEYBOARD,
        KEY_TYPE_MOUSE,
        KEY_TYPE_CONTROLLER_BTN,
        KEY_TYPE_CONTROLLER_AXIS
    };

    struct Keybind
    {
        uint32_t key;
        AxisMapping axisMapping;
        KeyType keyType;

        Keybind() : key((uint32_t)KEY_UNKNOWN), axisMapping(AXIS_MAPPING_FULL), keyType(KEY_TYPE_KEYBOARD) {}
        Keybind(Key key) : key((uint32_t)key), axisMapping(AXIS_MAPPING_FULL), keyType(KEY_TYPE_KEYBOARD) {}
        Keybind(MouseBtn key) : key((uint32_t)key), axisMapping(AXIS_MAPPING_FULL), keyType(KEY_TYPE_MOUSE) {}
        Keybind(ControllerBtn key) : key((uint32_t)key), axisMapping(AXIS_MAPPING_FULL), keyType(KEY_TYPE_CONTROLLER_BTN) {}
        Keybind(ControllerAxis key) : key((uint32_t)key.index), axisMapping(key.mapping), keyType(KEY_TYPE_CONTROLLER_AXIS) {}

        float getValue() const
        {
            if (key == KEY_UNKNOWN)
                return 0.0f;
            switch (keyType)
            {
            case KEY_TYPE_KEYBOARD:
                return Input::isDown((Key)key) ? 1.0f : 0.0f;
            case KEY_TYPE_MOUSE:
                return Input::isDown((MouseBtn)key) ? 1.0f : 0.0f;
            case KEY_TYPE_CONTROLLER_BTN:
                return Input::isDown((ControllerBtn)key) ? 1.0f : 0.0f;
            case KEY_TYPE_CONTROLLER_AXIS:
                return Input::getAxis(ControllerAxis(key, axisMapping));
            default:
                return 0.0f;
            }
        }

        bool isAxis() const
        {
            return keyType == KEY_TYPE_CONTROLLER_AXIS;
        }

        bool isDown() const
        {
            if (key == KEY_UNKNOWN)
                return false;
            switch (keyType)
            {
            case KEY_TYPE_KEYBOARD:
                return Input::isDown((Key)key);
            case KEY_TYPE_MOUSE:
                return Input::isDown((MouseBtn)key);
            case KEY_TYPE_CONTROLLER_BTN:
                return Input::isDown((ControllerBtn)key);
            default:
                return false;
            }
        }

        bool isUp() const
        {
            if (key == KEY_UNKNOWN)
                return true;
            switch (keyType)
            {
            case KEY_TYPE_KEYBOARD:
                return Input::isUp((Key)key);
            case KEY_TYPE_MOUSE:
                return Input::isUp((MouseBtn)key);
            case KEY_TYPE_CONTROLLER_BTN:
                return Input::isUp((ControllerBtn)key);
            default:
                return true;
            }
        }

        bool isPressed() const
        {
            if (key == KEY_UNKNOWN)
                return false;
            switch (keyType)
            {
            case KEY_TYPE_KEYBOARD:
                return Input::isPressed((Key)key);
            case KEY_TYPE_MOUSE:
                return Input::isPressed((MouseBtn)key);
            case KEY_TYPE_CONTROLLER_BTN:
                return Input::isPressed((ControllerBtn)key);
            default:
                return false;
            }
        }

        bool isReleased() const
        {
            if (key == KEY_UNKNOWN)
                return false;
            switch (keyType)
            {
            case KEY_TYPE_KEYBOARD:
                return Input::isReleased((Key)key);
            case KEY_TYPE_MOUSE:
                return Input::isReleased((MouseBtn)key);
            case KEY_TYPE_CONTROLLER_BTN:
                return Input::isReleased((ControllerBtn)key);
            default:
                return false;
            }
        }

        float getAxis() const
        {
            if (keyType == KEY_TYPE_CONTROLLER_AXIS)
                return Input::getAxis(ControllerAxis(key, axisMapping));
            else
                return 0;
        }
    };

    constexpr ControllerAxis CONTROLLER_AXIS_LX_FULL = {GLFW_GAMEPAD_AXIS_LEFT_X, AXIS_MAPPING_FULL};
    constexpr ControllerAxis CONTROLLER_AXIS_LX_POSITIVE = {GLFW_GAMEPAD_AXIS_LEFT_X, AXIS_MAPPING_POSITIVE_HALF};
    constexpr ControllerAxis CONTROLLER_AXIS_LX_NEGATIVE = {GLFW_GAMEPAD_AXIS_LEFT_X, AXIS_MAPPING_NEGATIVE_HALF};

    constexpr ControllerAxis CONTROLLER_AXIS_LY_FULL = {GLFW_GAMEPAD_AXIS_LEFT_Y, AXIS_MAPPING_FULL};
    constexpr ControllerAxis CONTROLLER_AXIS_LY_POSITIVE = {GLFW_GAMEPAD_AXIS_LEFT_Y, AXIS_MAPPING_POSITIVE_HALF};
    constexpr ControllerAxis CONTROLLER_AXIS_LY_NEGATIVE = {GLFW_GAMEPAD_AXIS_LEFT_Y, AXIS_MAPPING_NEGATIVE_HALF};

    constexpr ControllerAxis CONTROLLER_AXIS_RX_FULL = {GLFW_GAMEPAD_AXIS_RIGHT_X, AXIS_MAPPING_FULL};
    constexpr ControllerAxis CONTROLLER_AXIS_RX_POSITIVE = {GLFW_GAMEPAD_AXIS_RIGHT_X, AXIS_MAPPING_POSITIVE_HALF};
    constexpr ControllerAxis CONTROLLER_AXIS_RX_NEGATIVE = {GLFW_GAMEPAD_AXIS_RIGHT_X, AXIS_MAPPING_NEGATIVE_HALF};

    constexpr ControllerAxis CONTROLLER_AXIS_RY_FULL = {GLFW_GAMEPAD_AXIS_RIGHT_Y, AXIS_MAPPING_FULL};
    constexpr ControllerAxis CONTROLLER_AXIS_RY_POSITIVE = {GLFW_GAMEPAD_AXIS_RIGHT_Y, AXIS_MAPPING_POSITIVE_HALF};
    constexpr ControllerAxis CONTROLLER_AXIS_RY_NEGATIVE = {GLFW_GAMEPAD_AXIS_RIGHT_Y, AXIS_MAPPING_NEGATIVE_HALF};

    constexpr ControllerAxis CONTROLLER_AXIS_LT = {GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, AXIS_MAPPING_FULL};
    constexpr ControllerAxis CONTROLLER_AXIS_RT = {GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, AXIS_MAPPING_FULL};

    constexpr ControllerBtn CONTROLLER_BTN_A = GLFW_GAMEPAD_BUTTON_A;
    constexpr ControllerBtn CONTROLLER_BTN_B = GLFW_GAMEPAD_BUTTON_B;
    constexpr ControllerBtn CONTROLLER_BTN_X = GLFW_GAMEPAD_BUTTON_X;
    constexpr ControllerBtn CONTROLLER_BTN_Y = GLFW_GAMEPAD_BUTTON_Y;
    constexpr ControllerBtn CONTROLLER_BTN_LB = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;
    constexpr ControllerBtn CONTROLLER_BTN_RB = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;
    constexpr ControllerBtn CONTROLLER_BTN_BACK = GLFW_GAMEPAD_BUTTON_BACK;
    constexpr ControllerBtn CONTROLLER_BTN_START = GLFW_GAMEPAD_BUTTON_START;
    constexpr ControllerBtn CONTROLLER_BTN_GUIDE = GLFW_GAMEPAD_BUTTON_GUIDE;
    constexpr ControllerBtn CONTROLLER_BTN_LS = GLFW_GAMEPAD_BUTTON_LEFT_THUMB;
    constexpr ControllerBtn CONTROLLER_BTN_RS = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB;
    constexpr ControllerBtn CONTROLLER_BTN_DPAD_UP = GLFW_GAMEPAD_BUTTON_DPAD_UP;
    constexpr ControllerBtn CONTROLLER_BTN_DPAD_RIGHT = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT;
    constexpr ControllerBtn CONTROLLER_BTN_DPAD_DOWN = GLFW_GAMEPAD_BUTTON_DPAD_DOWN;
    constexpr ControllerBtn CONTROLLER_BTN_DPAD_LEFT = GLFW_GAMEPAD_BUTTON_DPAD_LEFT;

}