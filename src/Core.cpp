// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "VulkanManager.hpp"
#include "WindowManager.hpp"
#include "Input.hpp"
#include "Log.hpp"
#include "FPSManager.hpp"
#include "version.hpp"
#include "definitions.hpp"
#include "local.hpp"

#include "Vehicle.hpp"

class VergeEngine
{
public:
    VergeEngine() : vulkan(window.getWindowReference(), window.getWindowSize())
    {
        Input::init(window.getWindowReference());
        Log::add('C', 000);
    }

    void run()
    {
        VE_STRUCT_VEHICLE_CREATE_INFO sCar = {};
        sCar.power = 190;
        sCar.powerUnit = VE_POWER_UNIT_HORSEPOWER;
        sCar.weightKg = 1540;
        sCar.maxRpm = 7000;
        sCar.gearCount = 8;
        sCar.isAutomatic = true;
        sCar.accelerateKey = VE_KEY_W;
        sCar.turnLeftKey = VE_KEY_A;
        sCar.turnRightKey = VE_KEY_D;
        sCar.shiftUpKey = VE_KEY_M;
        sCar.shiftDownKey = VE_KEY_N;

        float ratios[8] = {5.519f, 3.184f, 2.050f, 1.492f, 1.235f, 1.000f, 0.801f, 0.673f};
        sCar.pGearRatios = ratios;

        sCar.finalDriveRatio = 3.2f;
        sCar.drivetrainEfficiency = 0.9f;
        sCar.wheelRadiusM = 0.31f;
        sCar.idleRpm = 800.0f;
        sCar.dragCoeff = 0.31f;
        sCar.frontalAreaM2 = 2.3f;
        car.init(sCar);

        while (window.isOpen())
        {
            fps.sync();
            Input::refresh();
            Log::printNewMessages();
            tick();
        }
    }

private:
    WindowManager window;
    VulkanManager vulkan;
    FpsManager fps;

    Vehicle car;

    void tick()
    {
        car.update(fps.getFrameTime());

        glm::mat4 firstModel(1.0f);
        glm::mat4 secondModel(1.0f);

        firstModel = glm::translate(firstModel, glm::vec3(-2.0f, 0.0f, -5.0f));
        firstModel = glm::rotate(firstModel, car.steeringAngleRad, glm::vec3(0.0, 0.0f, 1.0f));

        static float rotation = 0.0f;

        rotation += car.speedMps * fps.getFrameTime() / 10.0f * 500.0f;

        secondModel = glm::translate(secondModel, glm::vec3(2.0f, 0.0f, -5.0f));
        secondModel = glm::rotate(secondModel, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

        vulkan.updateModel(0, firstModel);
        vulkan.updateModel(1, secondModel);

        vulkan.drawFrame();
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