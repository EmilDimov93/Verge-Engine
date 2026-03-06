// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Input.hpp"

#include "Log.hpp"

KeyState Input::mouseBtnStates[VE_MOUSE_BTN_COUNT] = {};
KeyState Input::keyStates[VE_KEY_COUNT] = {};

Position2 Input::mousePosition = {0, 0};

int Input::controllerId = GLFW_JOYSTICK_1;
bool Input::controllerConnected = false;

std::vector<KeyState> Input::controllerBtnStates;
std::vector<float> Input::controllerAxes;

float Input::axisDeadZone = 0.001f;

GLFWwindow *Input::window = nullptr;

void Input::init(GLFWwindow *windowRef)
{
    window = windowRef;

    for (size_t i = 0; i < VE_MOUSE_BTN_COUNT; i++)
        mouseBtnStates[i] = KEY_STATE_UP;

    for (size_t i = 0; i < VE_KEY_COUNT; i++)
        keyStates[i] = KEY_STATE_UP;

    mousePosition = {0, 0};

    controllerConnected = false;
    controllerId = GLFW_JOYSTICK_1;
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

    controllerConnected = glfwJoystickPresent(controllerId);

    if(controllerConnected)
    {
        int axisCount = 0;
        const float *axisValues = glfwGetJoystickAxes(controllerId, &axisCount);

        if(controllerAxes.size() != axisCount)
            controllerAxes.resize(axisCount);
        
        for(int i = 0; i < axisCount; i++)
        {
            controllerAxes[i] = axisValues[i];
        }
        
        int buttonCount = 0;
        const unsigned char *buttonStates = glfwGetJoystickButtons(controllerId, &buttonCount);

        if(controllerBtnStates.size() != buttonCount)
            controllerBtnStates.resize(buttonCount);

        for(int i = 0; i < buttonCount; i++)
        {
            bool isDownNow = buttonStates[i] == GLFW_PRESS;

            switch (controllerBtnStates[i])
            {
            case KEY_STATE_UP:
            case KEY_STATE_RELEASED:
                controllerBtnStates[i] = isDownNow ? KEY_STATE_PRESSED : KEY_STATE_UP;
                break;
            case KEY_STATE_DOWN:
            case KEY_STATE_PRESSED:
                controllerBtnStates[i] = isDownNow ? KEY_STATE_DOWN : KEY_STATE_RELEASED;
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < controllerBtnStates.size(); i++)
        {
            if (controllerBtnStates[i] == KEY_STATE_DOWN || controllerBtnStates[i] == KEY_STATE_PRESSED)
                controllerBtnStates[i] = KEY_STATE_RELEASED;
            else
                controllerBtnStates[i] = KEY_STATE_UP;
        }

        for (int i = 0; i < controllerAxes.size(); i++)
            controllerAxes[i] = 0.0f;
    }
}

void Input::setController(uint8_t id)
{
    controllerId = id;
}

void Input::setAxisDeadZone(float deadZone)
{
    axisDeadZone = deadZone;
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

bool Input::isDown(VEControllerBtn btn)
{
    if (!controllerConnected || btn == VE_CONTROLLER_BTN_UNKNOWN)
        return false;
    if (btn >= controllerBtnStates.size() || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }

    return controllerBtnStates[btn] == KEY_STATE_DOWN || controllerBtnStates[btn] == KEY_STATE_PRESSED;
}

bool Input::isUp(VEControllerBtn btn)
{
    if (!controllerConnected || btn == VE_CONTROLLER_BTN_UNKNOWN)
        return false;
    if (btn >= controllerBtnStates.size() || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }

    return controllerBtnStates[btn] == KEY_STATE_UP || controllerBtnStates[btn] == KEY_STATE_RELEASED;
}

bool Input::isPressed(VEControllerBtn btn)
{
    if (!controllerConnected || btn == VE_CONTROLLER_BTN_UNKNOWN)
        return false;
    if (btn >= controllerBtnStates.size() || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }

    return controllerBtnStates[btn] == KEY_STATE_PRESSED;
}

bool Input::isReleased(VEControllerBtn btn)
{
    if (!controllerConnected || btn == VE_CONTROLLER_BTN_UNKNOWN)
        return false;
    if (btn >= controllerBtnStates.size() || btn < 0)
    {
        Log::add('G', 101);
        return false;
    }

    return controllerBtnStates[btn] == KEY_STATE_RELEASED;
}

float Input::getAxis(VEControllerAxis axis)
{
    if (!controllerConnected || axis.index == VE_CONTROLLER_AXIS_UNKNOWN.index)
        return 0.0f;
    if (axis.index >= controllerAxes.size() || axis.index < 0)
    {
        Log::add('G', 101);
        return 0.0f;
    }

    float axisValue = controllerAxes[axis.index];

    // Dead zone rescaling
    if (fabs(axisValue) < axisDeadZone)
        axisValue = 0.0f;
    else if (axisValue > 0.0f)
        axisValue = (axisValue - axisDeadZone) / (1.0f - axisDeadZone);
    else
        axisValue = (axisValue + axisDeadZone) / (1.0f - axisDeadZone);

    switch(axis.mapping)
    {
        case VE_AXIS_MAPPING_FULL:
            return (axisValue + 1.0f) / 2;
        case VE_AXIS_MAPPING_FULL_INVERTED:
            return 1.0f - (axisValue + 1.0f) / 2;
        case VE_AXIS_MAPPING_POSITIVE_HALF:
            return clamp01(axisValue);
        case VE_AXIS_MAPPING_NEGATIVE_HALF:
            return clamp01(-axisValue);
    }

    return 0.0f;
}
