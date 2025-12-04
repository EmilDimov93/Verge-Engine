// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "VulkanManager.hpp"
#include "WindowManager.hpp"
#include "Input.hpp"
#include "LogManager.hpp"
#include "FPSManager.hpp"
#include "version.hpp"
#include "definitions.hpp"
#include "local.hpp"

class VergeEngine
{
public:
    VergeEngine() : log(),
                    window(&log),
                    vulkan(window.getWindowReference(), window.getWindowSize(), &log),
                    fps(VE_DEFAULT_FPS)
    {
        Input::init(window.getWindowReference(), &log);
        log.add('C', 000);
    }

    void run()
    {
        while (window.isOpen())
            tick();
    }

private:
    LogManager log;
    WindowManager window;
    VulkanManager vulkan;
    FpsManager fps;

    float angle = 0.0f;
    float deltaTime = 0.0f;
    float lastTime = 0.0f;

    void tick()
    {
        Input::refresh();

        log.printNewMessages();

        float now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        angle += 200.0f * deltaTime;
        if(angle > 360.0f){
            angle -= 360.0f;
        }

        vulkan.updateModel(glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)));

        vulkan.drawFrame();

        fps.sync();
    }
};

int main()
{
    try
    {
        VergeEngine VE;

        VE.run();
    }
    catch (const EngineCrash &)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}