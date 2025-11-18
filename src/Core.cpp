// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include <iostream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <chrono>

#include "VulkanManager.hpp"
#include "InputManager.hpp"
#include "LogManager.hpp"
#include "FPSManager.hpp"
#include "version.hpp"
#include "definitions.hpp"
#include "local.hpp"

class VergeEngine
{
public:
    void run()
    {
        Init();

        while(!glfwWindowShouldClose(window)){
            Tick();
        }

        Cleanup();
    }

private:
    VulkanManager vulkan;

    GLFWwindow *window;
    Size windowSize = {0, 0};

    LogManager log;

    InputManager input;

    FpsManager fpsManager;

    void Init()
    {
        if (DEVELOPER_MODE)
        {
            log.add('O', 000);
        }

        if (!glfwInit())
        {
            log.add('G', 200);
        }

        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        windowSize.w = mode->width / 2;
        windowSize.h = mode->height / 2;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(windowSize.w, windowSize.h, "Verge Engine", nullptr, nullptr);
        if (!window)
        {
            log.add('G', 201);
        }

        log.add('G', 000);

        vulkan.initVulkan(window, windowSize, &log);

        input.initInputManager(&log);
        fpsManager.setTargetFps(140);

        log.add('C', 000);
    }

    void Tick()
    {
        input.refresh(window);

        if (log.hasNewMessages())
        {
            std::vector<std::string> newMessages = log.getNewMessages();

            for (size_t i = 0; i < newMessages.size(); i++)
            {
                std::cout << "LOG: " << newMessages[i] << std::endl;
            }
        }

        vulkan.drawFrame();

        fpsManager.syncFrameTime();
    }

    void Cleanup()
    {
        vulkan.cleanup();

        glfwDestroyWindow(window);
        glfwTerminate();

        log.add('C', 001);
        log.writeToLogFile();
    }
};

int main()
{
    VergeEngine verge;

    verge.run();

    return EXIT_SUCCESS;
}