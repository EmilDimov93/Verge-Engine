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
    void run()
    {
        init();

        while(window.shouldNotClose())
            tick();

        cleanup();
    }

private:
    VulkanManager vulkan;

    WindowManager window;

    LogManager log;

    InputManager input;

    FpsManager fps;

    void init()
    {
        if (DEVELOPER_MODE)
            log.add('O', 000);

        window.init(&log);

        vulkan.init(window.getWindowReference(), window.getWindowSize(), &log);

        input.init(window.getWindowReference(), &log);

        fps.setTarget(140);

        log.add('C', 000);
    }

    void tick()
    {
        input.refresh();

        log.printNewMessages();

        vulkan.drawFrame();

        fps.sync();
    }

    void cleanup()
    {
        vulkan.cleanup();

        window.cleanup();

        log.cleanup();
    }
};

int main()
{
    VergeEngine verge;

    verge.run();

    return EXIT_SUCCESS;
}