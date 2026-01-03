// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../Log.hpp"
#include "../version.hpp"
#include "../definitions.hpp"
#include "../local.hpp"

#include "../system/WindowManager.hpp"
#include "../system/Input.hpp"
#include "../system/FPSManager.hpp"

#include "../rendering/VulkanManager.hpp"

#include "../scene/Scene.hpp"

class VergeEngine
{
public:
    VergeEngine() : window(50.0f),
                    vulkan(window.getReference(), window.getSize()),
                    scene(vulkan.getContext(), window.getAspectRatio())
    {
        Input::init(window.getReference());
        Log::init(VE_LOG_OUTPUT_MODE_FILE_AND_CONSOLE);
    }

    void run()
    {
        setupScene();

        while (window.isOpen())
        {
            fps.sync();
            
            Input::refresh();

            scene.tick(fps.getFrameTime());

            vulkan.drawFrame(scene.meshes, scene.meshInstances, Camera::getProjectionMatrix(), Camera::getViewMatrix());
        }

        Log::end();
    }

private:
    WindowManager window;
    VulkanManager vulkan;
    FpsManager fps;

    Scene scene;

    void setupScene()
    {
        VE_STRUCT_VEHICLE_CREATE_INFO sCar = {};
        sCar.bodyMeshIndex = scene.loadFile("models/car.obj");
        sCar.wheelMeshIndex = scene.loadFile("models/wheel.obj");
        sCar.wheelOffset = {1.05f, 0.5f, 1.8f};
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
        sCar.brakingForce = 14700;
        sCar.tireGrip = 1.5f;
        sCar.camberRad = (PI / 18);
        scene.addVehicle(sCar, {{0, 2.9f, -100.0f}});

        scene.addProp(scene.loadFile("models/cow.obj"), {{10.0f, 3.0f, 30.0f}});

        VE_STRUCT_TRIGGER_TYPE_CREATE_INFO sTriggerType = {};
        sTriggerType.meshIndex = scene.loadFile("models/checkpoint.obj");
        sTriggerType.hitboxShape = VE_SHAPE_SPHERE;
        sTriggerType.hitboxSize = 10.0f;
        sTriggerType.isAutoDestroy = true;

        scene.addTrigger(0, sTriggerType, {{0, 0, 20.0f}, {0, PI / 2, 0}, {2.0f, 2.0f, 2.0f}});
        scene.addTrigger(1, sTriggerType, {{0, 0, 50.0f}, {0, PI / 2, 0}, {2.0f, 2.0f, 2.0f}});

        scene.setCameraFollowVehicle(0);

        uint32_t grassSurfaceIndex = scene.addSurface({0.4f, {0, 0.6f, 0}, {0, 0.05f, 0}, 0.2f});

        uint32_t asphaltSurfaceIndex = scene.addSurface({1.0f, {0.2f, 0.2f, 0.2f}, {0.01f, 0.0f, 0.0f}});

        scene.buildGroundMesh({1000, 1000});
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