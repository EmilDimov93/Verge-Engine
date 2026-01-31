// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Input.hpp"

#include "Log.hpp"

#define AXIS_DEAD_ZONE 0.001f

KeyState Input::mouseBtnStates[VE_MOUSE_BTN_COUNT] = {};
KeyState Input::keyStates[VE_KEY_COUNT] = {};

Position2 Input::mousePosition = {0, 0};

int Input::gamepadId = GLFW_JOYSTICK_1;
bool Input::gamepadConnected = false;

KeyState Input::gamepadBtnStates[VE_GAMEPAD_BTN_COUNT] = {};
float Input::gamepadAxes[VE_GAMEPAD_AXIS_COUNT] = {};

GLFWwindow *Input::window = nullptr;

void Input::init(GLFWwindow *windowRef)
{
    window = windowRef;

    for (size_t i = 0; i < VE_MOUSE_BTN_COUNT; i++)
        mouseBtnStates[i] = KEY_STATE_UP;

    for (size_t i = 0; i < VE_KEY_COUNT; i++)
        keyStates[i] = KEY_STATE_UP;

    mousePosition = {0, 0};

    for (size_t i = 0; i < VE_GAMEPAD_BTN_COUNT; i++)
        gamepadBtnStates[i] = KEY_STATE_UP;

    for (size_t i = 0; i < VE_GAMEPAD_AXIS_COUNT; i++)
        gamepadAxes[i] = -1.0f;

    gamepadConnected = false;
    gamepadId = GLFW_JOYSTICK_1;
}

void Input::refresh()
{
    glfwPollEvents();

    for (size_t i = 0; i < VE_MOUSE_BTN_COUNT; i++)
    {
        bool isKeyDown = glfwGetMouseButton(window, i) == GLFW_PRESS;

        switch (mouseBtnStates[i])
        {
        case KEY_STATE_UP:
        case KEY_STATE_RELEASED:
            mouseBtnStates[i] = isKeyDown ? KEY_STATE_PRESSED : KEY_STATE_UP;
            break;
        case KEY_STATE_DOWN:
        case KEY_STATE_PRESSED:
            mouseBtnStates[i] = isKeyDown ? KEY_STATE_DOWN : KEY_STATE_RELEASED;
            break;
        }
    }

    for (size_t i = 0; i < VE_KEY_COUNT; i++)
    {
        bool isKeyDown = glfwGetKey(window, i) == GLFW_PRESS;

        switch (keyStates[i])
        {
        case KEY_STATE_UP:
        case KEY_STATE_RELEASED:
            keyStates[i] = isKeyDown ? KEY_STATE_PRESSED : KEY_STATE_UP;
            break;
        case KEY_STATE_DOWN:
        case KEY_STATE_PRESSED:
            keyStates[i] = isKeyDown ? KEY_STATE_DOWN : KEY_STATE_RELEASED;
            break;
        }
    }

    glfwGetCursorPos(window, &mousePosition.x, &mousePosition.y);

    gamepadConnected = glfwJoystickPresent(gamepadId) && glfwJoystickIsGamepad(gamepadId);

    GLFWgamepadstate state{};
    if (gamepadConnected && glfwGetGamepadState(gamepadId, &state))
    {
        for (int i = 0; i < VE_GAMEPAD_BTN_COUNT; i++)
        {
            bool isDownNow = state.buttons[i] == GLFW_PRESS;

            switch (gamepadBtnStates[i])
            {
            case KEY_STATE_UP:
            case KEY_STATE_RELEASED:
                gamepadBtnStates[i] = isDownNow ? KEY_STATE_PRESSED : KEY_STATE_UP;
                break;
            case KEY_STATE_DOWN:
            case KEY_STATE_PRESSED:
                gamepadBtnStates[i] = isDownNow ? KEY_STATE_DOWN : KEY_STATE_RELEASED;
                break;
            }
        }

        float lx = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        float ly = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

        if (fabsf(lx) < AXIS_DEAD_ZONE)
            lx = 0.0f;
        if (fabsf(ly) < AXIS_DEAD_ZONE)
            ly = 0.0f;

        gamepadAxes[VE_GAMEPAD_AXIS_LX_POS] = (lx > 0.0f) ? lx : 0.0f;
        gamepadAxes[VE_GAMEPAD_AXIS_LX_NEG] = (lx < 0.0f) ? -lx : 0.0f;
        gamepadAxes[VE_GAMEPAD_AXIS_LY_POS] = (ly > 0.0f) ? ly : 0.0f;
        gamepadAxes[VE_GAMEPAD_AXIS_LY_NEG] = (ly < 0.0f) ? -ly : 0.0f;

        float rx = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        float ry = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];

        if (fabsf(rx) < AXIS_DEAD_ZONE)
            rx = 0.0f;
        if (fabsf(ry) < AXIS_DEAD_ZONE)
            ry = 0.0f;

        gamepadAxes[VE_GAMEPAD_AXIS_RX_POS] = (rx > 0.0f) ? rx : 0.0f;
        gamepadAxes[VE_GAMEPAD_AXIS_RX_NEG] = (rx < 0.0f) ? -rx : 0.0f;
        gamepadAxes[VE_GAMEPAD_AXIS_RY_POS] = (ry > 0.0f) ? ry : 0.0f;
        gamepadAxes[VE_GAMEPAD_AXIS_RY_NEG] = (ry < 0.0f) ? -ry : 0.0f;

        float lt = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
        float rt = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];

        lt = (lt + 1.0f) * 0.5f;
        if (lt < 0.0f)
            lt = 0.0f;
        if (lt > 1.0f)
            lt = 1.0f;

        rt = (rt + 1.0f) * 0.5f;
        if (rt < 0.0f)
            rt = 0.0f;
        if (rt > 1.0f)
            rt = 1.0f;

        gamepadAxes[VE_GAMEPAD_AXIS_LT] = lt;
        gamepadAxes[VE_GAMEPAD_AXIS_RT] = rt;
    }
    else
    {
        for (int i = 0; i < VE_GAMEPAD_BTN_COUNT; i++)
        {
            if (gamepadBtnStates[i] == KEY_STATE_DOWN || gamepadBtnStates[i] == KEY_STATE_PRESSED)
                gamepadBtnStates[i] = KEY_STATE_RELEASED;
            else
                gamepadBtnStates[i] = KEY_STATE_UP;
        }

        for (int i = 0; i < VE_GAMEPAD_AXIS_COUNT; i++)
            gamepadAxes[i] = 0.0f;
    }
}

void Input::setGamepad(uint8_t id)
{
    gamepadId = id;
}

bool Input::isDown(VEKey key)
{
    if (key == VE_KEY_UNKNOWN)
        return false;
    if (key >= VE_KEY_COUNT || key < 0)
    {
        Log::add('G', 101);
        return false;
    }
    return keyStates[key] == KEY_STATE_DOWN || keyStates[key] == KEY_STATE_PRESSED;
}

bool Input::isUp(VEKey key)
{
    if (key == VE_KEY_UNKNOWN)
        return false;
    if (key >= VE_KEY_COUNT || key < 0)
    {
        Log::add('G', 101);
        return false;
    }
    return keyStates[key] == KEY_STATE_UP || keyStates[key] == KEY_STATE_RELEASED;
}

bool Input::isPressed(VEKey key)
{
    if (key == VE_KEY_UNKNOWN)
        return false;
    if (key >= VE_KEY_COUNT || key < 0)
    {
        Log::add('G', 101);
        return false;
    }
    return keyStates[key] == KEY_STATE_PRESSED;
}

bool Input::isReleased(VEKey key)
{
    if (key == VE_KEY_UNKNOWN)
        return false;
    if (key >= VE_KEY_COUNT || key < 0)
    {
        Log::add('G', 101);
        return false;
    }
    return keyStates[key] == KEY_STATE_RELEASED;
}

bool Input::isDown(VEMouseBtn btn)
{
    if (btn == VE_MOUSE_BTN_UNKNOWN)
        return false;
    if (btn >= VE_MOUSE_BTN_COUNT || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_DOWN || mouseBtnStates[btn] == KEY_STATE_PRESSED;
}

bool Input::isUp(VEMouseBtn btn)
{
    if (btn == VE_MOUSE_BTN_UNKNOWN)
        return false;
    if (btn >= VE_MOUSE_BTN_COUNT || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_UP || mouseBtnStates[btn] == KEY_STATE_RELEASED;
}

bool Input::isPressed(VEMouseBtn btn)
{
    if (btn == VE_MOUSE_BTN_UNKNOWN)
        return false;
    if (btn >= VE_MOUSE_BTN_COUNT || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_PRESSED;
}

bool Input::isReleased(VEMouseBtn btn)
{
    if (btn == VE_MOUSE_BTN_UNKNOWN)
        return false;
    if (btn >= VE_MOUSE_BTN_COUNT || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_RELEASED;
}

Position2 Input::getMousePos()
{
    return mousePosition;
}

bool Input::isDown(VEGamepadBtn btn)
{
    if (!gamepadConnected || btn == VE_GAMEPAD_BTN_UNKNOWN)
        return false;
    if (btn >= VE_MOUSE_BTN_COUNT || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }

    return gamepadBtnStates[btn] == KEY_STATE_DOWN || gamepadBtnStates[btn] == KEY_STATE_PRESSED;
}

bool Input::isUp(VEGamepadBtn btn)
{
    if (!gamepadConnected || btn == VE_GAMEPAD_BTN_UNKNOWN)
        return true;
    if (btn >= VE_MOUSE_BTN_COUNT || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }

    return gamepadBtnStates[btn] == KEY_STATE_UP || gamepadBtnStates[btn] == KEY_STATE_RELEASED;
}

bool Input::isPressed(VEGamepadBtn btn)
{
    if (!gamepadConnected || btn == VE_GAMEPAD_BTN_UNKNOWN)
        return false;
    if (btn >= VE_MOUSE_BTN_COUNT || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }

    return gamepadBtnStates[btn] == KEY_STATE_PRESSED;
}

bool Input::isReleased(VEGamepadBtn btn)
{
    if (!gamepadConnected || btn == VE_GAMEPAD_BTN_UNKNOWN)
        return false;
    if (btn >= VE_MOUSE_BTN_COUNT || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }

    return gamepadBtnStates[btn] == KEY_STATE_RELEASED;
}

float Input::getAxis(VEGamepadAxis axis)
{
    if (!gamepadConnected || axis == VE_GAMEPAD_AXIS_UNKNOWN)
        return 0.0f;
    if (axis >= VE_GAMEPAD_AXIS_COUNT || axis < 0)
    {
        Log::add('G', 101);
        return 0.0f;
    }

    return gamepadAxes[axis];
}