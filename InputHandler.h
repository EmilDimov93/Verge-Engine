#pragma once

#include <GLFW/glfw3.h>

typedef enum{
    KEY_STATE_DOWN,
    KEY_STATE_UP,
    KEY_STATE_PRESSED,
    KEY_STATE_RELEASED
}KeyState;

class InputHandler{
    private:
        
    public:bool LMBState;
        void RefreshInput(GLFWwindow *window);

        void IsLMBPressed();
};