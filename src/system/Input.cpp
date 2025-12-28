// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Input.hpp"

#include "../Log.hpp"

KeyState Input::mouseBtnStates[VE_MOUSE_BTN_COUNT] = {};
KeyState Input::keyStates[VE_KEY_COUNT] = {};

Position2 Input::mousePosition = {0, 0};

GLFWwindow *Input::window = nullptr;

void Input::init(GLFWwindow *windowRef)
{
    window = windowRef;

    for (size_t i = 0; i < VE_MOUSE_BTN_COUNT; i++)
    {
        mouseBtnStates[i] = KEY_STATE_UP;
    }

    for (size_t i = 0; i < VE_KEY_COUNT; i++)
    {
        keyStates[i] = KEY_STATE_UP;
    }

    mousePosition = {0, 0};
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
}

bool Input::isDown(VEMouseBtn btn)
{
    if (btn == VE_MOUSE_BTN_UNKNOWN)
        return false;
    if (btn > VE_MOUSE_BTN_COUNT)
    {
        Log::add('C', 100);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_DOWN || mouseBtnStates[btn] == KEY_STATE_PRESSED;
}

bool Input::isUp(VEMouseBtn btn)
{
    if (btn == VE_MOUSE_BTN_UNKNOWN)
        return false;
    if (btn > VE_MOUSE_BTN_COUNT)
    {
        Log::add('C', 100);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_UP || mouseBtnStates[btn] == KEY_STATE_RELEASED;
}

bool Input::isPressed(VEMouseBtn btn)
{
    if (btn == VE_MOUSE_BTN_UNKNOWN)
        return false;
    if (btn > VE_MOUSE_BTN_COUNT)
    {
        Log::add('C', 100);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_PRESSED;
}

bool Input::isReleased(VEMouseBtn btn)
{
    if (btn == VE_MOUSE_BTN_UNKNOWN)
        return false;
    if (btn > VE_MOUSE_BTN_COUNT)
    {
        Log::add('C', 100);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_RELEASED;
}

bool Input::isDown(VEKey key)
{
    if (key == VE_KEY_UNKNOWN)
        return false;
    if (key > VE_KEY_COUNT)
    {
        Log::add('C', 100);
        return false;
    }
    return keyStates[key] == KEY_STATE_DOWN || keyStates[key] == KEY_STATE_PRESSED;
}

bool Input::isUp(VEKey key)
{
    if (key == VE_KEY_UNKNOWN)
        return false;
    if (key > VE_KEY_COUNT)
    {
        Log::add('C', 100);
        return false;
    }
    return keyStates[key] == KEY_STATE_UP || keyStates[key] == KEY_STATE_RELEASED;
}

bool Input::isPressed(VEKey key)
{
    if (key == VE_KEY_UNKNOWN)
        return false;
    if (key > VE_KEY_COUNT)
    {
        Log::add('C', 100);
        return false;
    }
    return keyStates[key] == KEY_STATE_PRESSED;
}

bool Input::isReleased(VEKey key)
{
    if (key == VE_KEY_UNKNOWN)
        return false;
    if (key > VE_KEY_COUNT)
    {
        Log::add('C', 100);
        return false;
    }
    return keyStates[key] == KEY_STATE_RELEASED;
}

Position2 Input::getMousePos()
{
    return mousePosition;
}