#include "InputManager.h"

InputManager::InputManager()
{
    for (int i = 0; i < VRG_MOUSE_KEY_COUNT; i++)
    {
        mouseKeyStates[i] = KEY_STATE_UP;
    }

    for (int i = 0; i < VRG_KEY_COUNT; i++)
    {
        keyStates[i] = KEY_STATE_UP;
    }
}

void InputManager::refresh(GLFWwindow *window)
{
    for (int i = 0; i < VRG_MOUSE_KEY_COUNT; i++)
    {
        bool isKeyDown = glfwGetMouseButton(window, i) == GLFW_PRESS;

        switch (mouseKeyStates[i])
        {
        case KEY_STATE_UP:
        case KEY_STATE_RELEASED:
            mouseKeyStates[i] = isKeyDown ? KEY_STATE_PRESSED : KEY_STATE_UP;
            break;
        case KEY_STATE_DOWN:
        case KEY_STATE_PRESSED:
            mouseKeyStates[i] = isKeyDown ? KEY_STATE_DOWN : KEY_STATE_RELEASED;
            break;
        }
    }

    for (int i = 0; i < VRG_KEY_COUNT; i++)
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

bool InputManager::isMouseKeyDown(int key)
{
    return mouseKeyStates[key] == KEY_STATE_DOWN || mouseKeyStates[key] == KEY_STATE_PRESSED;
}

bool InputManager::isMouseKeyUp(int key)
{
    return mouseKeyStates[key] == KEY_STATE_UP || mouseKeyStates[key] == KEY_STATE_RELEASED;
}

bool InputManager::isMouseKeyPressed(int key)
{
    return mouseKeyStates[key] == KEY_STATE_PRESSED;
}

bool InputManager::isMouseKeyReleased(int key)
{
    return mouseKeyStates[key] == KEY_STATE_RELEASED;
}

bool InputManager::isKeyDown(int key)
{
    return keyStates[key] == KEY_STATE_DOWN || keyStates[key] == KEY_STATE_PRESSED;
}

bool InputManager::isKeyUp(int key)
{
    return keyStates[key] == KEY_STATE_UP || keyStates[key] == KEY_STATE_RELEASED;
}

bool InputManager::isKeyPressed(int key)
{
    return keyStates[key] == KEY_STATE_PRESSED;
}

bool InputManager::isKeyReleased(int key)
{
    return keyStates[key] == KEY_STATE_RELEASED;
}