// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "VulkanManager.hpp"
#include "WindowManager.hpp"
#include "InputManager.hpp"
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
                    input(window.getWindowReference(), &log),
                    fps(VE_DEFAULT_FPS)
    {
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
    InputManager input;
    FpsManager fps;

    void tick()
    {
        input.refresh();

        log.printNewMessages();

        vulkan.drawFrame();

        fps.sync();
    }
};

int main()
{
    VergeEngine VE;

    VE.run();
}