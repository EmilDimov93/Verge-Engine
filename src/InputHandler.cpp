#include "InputHandler.h"

InputHandler::InputHandler()
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

void InputHandler::RefreshInput(GLFWwindow *window)
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

bool InputHandler::IsMouseKeyDown(int key)
{
    return mouseKeyStates[key] == KEY_STATE_DOWN || mouseKeyStates[key] == KEY_STATE_PRESSED;
}

bool InputHandler::IsMouseKeyUp(int key)
{
    return mouseKeyStates[key] == KEY_STATE_UP || mouseKeyStates[key] == KEY_STATE_RELEASED;
}

bool InputHandler::IsMouseKeyPressed(int key)
{
    return mouseKeyStates[key] == KEY_STATE_PRESSED;
}

bool InputHandler::IsMouseKeyReleased(int key)
{
    return mouseKeyStates[key] == KEY_STATE_RELEASED;
}

bool InputHandler::IsKeyDown(int key)
{
    return keyStates[key] == KEY_STATE_DOWN || keyStates[key] == KEY_STATE_PRESSED;
}

bool InputHandler::IsKeyUp(int key)
{
    return keyStates[key] == KEY_STATE_UP || keyStates[key] == KEY_STATE_RELEASED;
}

bool InputHandler::IsKeyPressed(int key)
{
    return keyStates[key] == KEY_STATE_PRESSED;
}

bool InputHandler::IsKeyReleased(int key)
{
    return keyStates[key] == KEY_STATE_RELEASED;
}