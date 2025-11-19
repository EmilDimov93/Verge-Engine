// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "InputManager.hpp"

#include "LogManager.hpp"

void InputManager::init(GLFWwindow *windowRef, LogManager *logRef)
{
    window = windowRef;
    log = logRef;

    for (size_t i = 0; i < VE_MOUSE_BTN_COUNT; i++)
    {
        mouseBtnStates[i] = KEY_STATE_UP;
    }

    for (size_t i = 0; i < VE_KEY_COUNT; i++)
    {
        keyStates[i] = KEY_STATE_UP;
    }
}

void InputManager::refresh()
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
}

bool InputManager::isDown(VEMouseBtn btn)
{
    if(btn > VE_MOUSE_BTN_COUNT){
        log->add('C', 100);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_DOWN || mouseBtnStates[btn] == KEY_STATE_PRESSED;
}

bool InputManager::isUp(VEMouseBtn btn)
{
    if(btn > VE_MOUSE_BTN_COUNT){
        log->add('C', 100);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_UP || mouseBtnStates[btn] == KEY_STATE_RELEASED;
}

bool InputManager::isPressed(VEMouseBtn btn)
{
    if(btn > VE_MOUSE_BTN_COUNT){
        log->add('C', 100);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_PRESSED;
}

bool InputManager::isReleased(VEMouseBtn btn)
{
    if(btn > VE_MOUSE_BTN_COUNT){
        log->add('C', 100);
        return false;
    }
    return mouseBtnStates[btn] == KEY_STATE_RELEASED;
}

bool InputManager::isDown(VEKey key)
{
    if(key > VE_KEY_COUNT){
        log->add('C', 100);
        return false;
    }
    return keyStates[key] == KEY_STATE_DOWN || keyStates[key] == KEY_STATE_PRESSED;
}

bool InputManager::isUp(VEKey key)
{
    if(key > VE_KEY_COUNT){
        log->add('C', 100);
        return false;
    }
    return keyStates[key] == KEY_STATE_UP || keyStates[key] == KEY_STATE_RELEASED;
}

bool InputManager::isPressed(VEKey key)
{
    if(key > VE_KEY_COUNT){
        log->add('C', 100);
        return false;
    }
    return keyStates[key] == KEY_STATE_PRESSED;
}

bool InputManager::isReleased(VEKey key)
{
    if(key > VE_KEY_COUNT){
        log->add('C', 100);
        return false;
    }
    return keyStates[key] == KEY_STATE_RELEASED;
}