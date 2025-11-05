// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "InputManager.hpp"

InputManager::InputManager()
{
    for (size_t i = 0; i < VRG_MOUSE_KEY_COUNT; i++)
    {
        mouseBtnStates[i] = KEY_STATE_UP;
    }

    for (size_t i = 0; i < VRG_KEY_COUNT; i++)
    {
        keyStates[i] = KEY_STATE_UP;
    }
}

void InputManager::refresh(GLFWwindow *window)
{
    glfwPollEvents();
    
    for (size_t i = 0; i < VRG_MOUSE_KEY_COUNT; i++)
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

    for (size_t i = 0; i < VRG_KEY_COUNT; i++)
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
}

bool InputManager::isDown(VRGMouseBtn btn)
{
    return mouseBtnStates[btn] == KEY_STATE_DOWN || mouseBtnStates[btn] == KEY_STATE_PRESSED;
}

bool InputManager::isUp(VRGMouseBtn btn)
{
    return mouseBtnStates[btn] == KEY_STATE_UP || mouseBtnStates[btn] == KEY_STATE_RELEASED;
}

bool InputManager::isPressed(VRGMouseBtn btn)
{
    return mouseBtnStates[btn] == KEY_STATE_PRESSED;
}

bool InputManager::isReleased(VRGMouseBtn btn)
{
    return mouseBtnStates[btn] == KEY_STATE_RELEASED;
}

bool InputManager::isDown(VRGKey key)
{
    return keyStates[key] == KEY_STATE_DOWN || keyStates[key] == KEY_STATE_PRESSED;
}

bool InputManager::isUp(VRGKey key)
{
    return keyStates[key] == KEY_STATE_UP || keyStates[key] == KEY_STATE_RELEASED;
}

bool InputManager::isPressed(VRGKey key)
{
    return keyStates[key] == KEY_STATE_PRESSED;
}

bool InputManager::isReleased(VRGKey key)
{
    return keyStates[key] == KEY_STATE_RELEASED;
}