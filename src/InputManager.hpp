// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <GLFW/glfw3.h>

class LogManager;

typedef enum
{
    KEY_STATE_DOWN,
    KEY_STATE_UP,
    KEY_STATE_PRESSED,
    KEY_STATE_RELEASED
} KeyState;

#define VE_MOUSE_BTN_COUNT GLFW_MOUSE_BUTTON_LAST
#define VE_KEY_COUNT GLFW_KEY_LAST

typedef enum
{
    VE_MOUSE_BTN_LEFT = GLFW_MOUSE_BUTTON_1,
    VE_MOUSE_BTN_RIGHT = GLFW_MOUSE_BUTTON_2,
    VE_MOUSE_BTN_MIDDLE = GLFW_MOUSE_BUTTON_3,
    VE_MOUSE_BTN_4 = GLFW_MOUSE_BUTTON_4,
    VE_MOUSE_BTN_5 = GLFW_MOUSE_BUTTON_5,
    VE_MOUSE_BTN_6 = GLFW_MOUSE_BUTTON_6,
    VE_MOUSE_BTN_7 = GLFW_MOUSE_BUTTON_7,
    VE_MOUSE_BTN_8 = GLFW_MOUSE_BUTTON_8
} VEMouseBtn;

typedef enum
{
    VE_KEY_A = GLFW_KEY_A,
    VE_KEY_B = GLFW_KEY_B,
    VE_KEY_C = GLFW_KEY_C,
    VE_KEY_D = GLFW_KEY_D,
    VE_KEY_E = GLFW_KEY_E,
    VE_KEY_F = GLFW_KEY_F,
    VE_KEY_G = GLFW_KEY_G,
    VE_KEY_H = GLFW_KEY_H,
    VE_KEY_I = GLFW_KEY_I,
    VE_KEY_J = GLFW_KEY_J,
    VE_KEY_K = GLFW_KEY_K,
    VE_KEY_L = GLFW_KEY_L,
    VE_KEY_M = GLFW_KEY_M,
    VE_KEY_N = GLFW_KEY_N,
    VE_KEY_O = GLFW_KEY_O,
    VE_KEY_P = GLFW_KEY_P,
    VE_KEY_Q = GLFW_KEY_Q,
    VE_KEY_R = GLFW_KEY_R,
    VE_KEY_S = GLFW_KEY_S,
    VE_KEY_T = GLFW_KEY_T,
    VE_KEY_U = GLFW_KEY_U,
    VE_KEY_V = GLFW_KEY_V,
    VE_KEY_W = GLFW_KEY_W,
    VE_KEY_X = GLFW_KEY_X,
    VE_KEY_Y = GLFW_KEY_Y,
    VE_KEY_Z = GLFW_KEY_Z,
    VE_KEY_0 = GLFW_KEY_0,
    VE_KEY_1 = GLFW_KEY_1,
    VE_KEY_2 = GLFW_KEY_2,
    VE_KEY_3 = GLFW_KEY_3,
    VE_KEY_4 = GLFW_KEY_4,
    VE_KEY_5 = GLFW_KEY_5,
    VE_KEY_6 = GLFW_KEY_6,
    VE_KEY_7 = GLFW_KEY_7,
    VE_KEY_8 = GLFW_KEY_8,
    VE_KEY_9 = GLFW_KEY_9,
    VE_KEY_F1 = GLFW_KEY_F1,
    VE_KEY_F2 = GLFW_KEY_F2,
    VE_KEY_F3 = GLFW_KEY_F3,
    VE_KEY_F4 = GLFW_KEY_F4,
    VE_KEY_F5 = GLFW_KEY_F5,
    VE_KEY_F6 = GLFW_KEY_F6,
    VE_KEY_F7 = GLFW_KEY_F7,
    VE_KEY_F8 = GLFW_KEY_F8,
    VE_KEY_F9 = GLFW_KEY_F9,
    VE_KEY_F10 = GLFW_KEY_F10,
    VE_KEY_F11 = GLFW_KEY_F11,
    VE_KEY_F12 = GLFW_KEY_F12,
    VE_KEY_SPACE = GLFW_KEY_SPACE,
    VE_KEY_ENTER = GLFW_KEY_ENTER,
    VE_KEY_ESCAPE = GLFW_KEY_ESCAPE,
    VE_KEY_TAB = GLFW_KEY_TAB,
    VE_KEY_BACKSPACE = GLFW_KEY_BACKSPACE,
    VE_KEY_INSERT = GLFW_KEY_INSERT,
    VE_KEY_DELETE = GLFW_KEY_DELETE,
    VE_KEY_RIGHT = GLFW_KEY_RIGHT,
    VE_KEY_LEFT = GLFW_KEY_LEFT,
    VE_KEY_UP = GLFW_KEY_UP,
    VE_KEY_DOWN = GLFW_KEY_DOWN,
    VE_KEY_HOME = GLFW_KEY_HOME,
    VE_KEY_END = GLFW_KEY_END,
    VE_KEY_PAGE_UP = GLFW_KEY_PAGE_UP,
    VE_KEY_PAGE_DOWN = GLFW_KEY_PAGE_DOWN,
    VE_KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
    VE_KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
    VE_KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
    VE_KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
    VE_KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT,
    VE_KEY_RIGHT_ALT = GLFW_KEY_RIGHT_ALT
} VEKey;

class InputManager
{
public:
    InputManager(GLFWwindow *windowRef, LogManager *logRef);

    void refresh();

    bool isDown(VEMouseBtn key);
    bool isUp(VEMouseBtn key);
    bool isPressed(VEMouseBtn key);
    bool isReleased(VEMouseBtn key);

    bool isDown(VEKey key);
    bool isUp(VEKey key);
    bool isPressed(VEKey key);
    bool isReleased(VEKey key);

private:
    KeyState mouseBtnStates[VE_MOUSE_BTN_COUNT];
    KeyState keyStates[VE_KEY_COUNT];

    GLFWwindow *window;

    LogManager *log;
};