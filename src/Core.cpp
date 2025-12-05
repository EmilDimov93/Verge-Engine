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

        angle += 10.0f * deltaTime;
        if(angle > 360.0f){
            angle -= 360.0f;
        }

        glm::mat4 firstModel(1.0f);
        glm::mat4 secondModel(1.0f);

        firstModel = glm::translate(firstModel, glm::vec3(-2.0f, 0.0f, -5.0f));
        firstModel = glm::rotate(firstModel, glm::radians(angle), glm::vec3(0.0, 0.0f, 1.0f));

        secondModel = glm::translate(secondModel, glm::vec3(2.0f, 0.0f, -5.0f));
        secondModel = glm::rotate(secondModel, glm::radians(-angle * 100), glm::vec3(0.0f, 0.0f, 1.0f));

        vulkan.updateModel(0, firstModel);
        vulkan.updateModel(1, secondModel);

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