// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include <iostream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <chrono>

#include "VulkanManager.hpp"
#include "GlfwManager.hpp"
#include "InputManager.hpp"
#include "LogManager.hpp"
#include "FPSManager.hpp"
#include "version.hpp"
#include "definitions.hpp"
#include "local.hpp"

const Size windowSize = {800, 600};

class VergeEngine
{
public:
    void run()
    {
        Init();
        Tick();
        Cleanup();
    }

private:
    VulkanManager vulkan;

    GlfwManager glfw;

    GLFWwindow *window;
    
    InputManager input;

    FpsManager fpsManager;

    LogManager log;

    void Init()
    {
        if(DEVELOPER_MODE){
            log.add('O', 000);
        }

        glfw.initWindow(&window, windowSize, &log);
        vulkan.initVulkan(window, windowSize, &log);

        fpsManager.setTargetFps(140);

        log.add('C', 000);
    }

    void Tick()
    {std::cout << "\033[31mRed text\033[0m Normal text\n";
        glfwPollEvents();
        input.refresh(window);

        if (log.hasNewMessages())
        {
            std::vector<std::string> newMessages = log.getNewMessages();

            for (size_t i = 0; i < newMessages.size(); i++)
            {
                std::cout << newMessages[i] << std::endl;
            }
        }

        vulkan.drawFrame();

        fpsManager.syncFrameTime();

        if (!glfwWindowShouldClose(window))
        {
            Tick();
        }
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
    try
    {
        verge.run();
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}