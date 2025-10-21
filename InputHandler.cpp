#include <GLFW/glfw3.h>

#include "InputHandler.h"

void InputHandler::RefreshInput(GLFWwindow* window) {
    LMBState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}