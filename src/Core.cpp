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
#include "Camera.hpp"
#include "Scene.hpp"

class VergeEngine
{
public:
    VergeEngine() : vulkan(window.getWindowReference(), window.getWindowSize()), scene(vulkan.getContext(), 60.0f, window.getAspectRatio(), 0.1f, 1000.0f)
    {
        Input::init(window.getWindowReference());
        Log::add('C', 000);
    }

    void run()
    {
        VE_STRUCT_VEHICLE_CREATE_INFO sCar = {};
        sCar.bodyMeshIndex = 0;
        sCar.wheelFLMeshIndex = 1;
        sCar.wheelFRMeshIndex = 2;
        sCar.wheelBLMeshIndex = 3;
        sCar.wheelBRMeshIndex = 4;
        sCar.power = 190;
        sCar.powerUnit = VE_POWER_UNIT_HORSEPOWER;
        sCar.weightKg = 1540;
        sCar.maxRpm = 7000;
        sCar.gearCount = 8;
        sCar.transmissionType = VE_TRANSMISSION_TYPE_AUTOMATIC;
        sCar.accelerateKey = VE_KEY_W;
        sCar.brakeKey = VE_KEY_S;
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
        scene.addVehicle(sCar);

        scene.loadFile("models/car3.obj", {0, 0, 0});

        scene.loadFile("models/wheel2.obj", {0, 0, 1.0f});
        scene.loadFile("models/wheel2.obj", {0, 0, 1.0f});
        scene.loadFile("models/wheel2.obj", {0, 0, 1.0f});
        scene.loadFile("models/wheel2.obj", {0, 0, 1.0f});

        scene.loadFile("models/flag.obj", {0, 1.0f, 0});
        scene.loadFile("models/flag.obj", {0, 1.0f, 0});

        scene.setCameraFollowVehicle(0);

        while (window.isOpen())
        {
            fps.sync();
            Input::refresh();
            Log::printNewMessages();
            tick();
        }

        Log::end();
    }

private:
    WindowManager window;
    VulkanManager vulkan;
    FpsManager fps;

    Scene scene;

    void tick()
    {
        glm::mat4 flag1Model(1.0f), flag2Model(1.0f);

        flag1Model = glm::translate(flag1Model, glm::vec3(2.0f, 0, 20.0f));
        flag2Model = glm::translate(flag2Model, glm::vec3(-2.0f, 0, 20.0f));

        scene.updateModel(5, flag1Model);
        scene.updateModel(6, flag2Model);

        scene.tick(fps.getFrameTime());

        vulkan.drawFrame(scene.meshes);
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