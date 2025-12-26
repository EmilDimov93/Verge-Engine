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

#include "../scene/camera/Camera.hpp"

class VergeEngine
{
public:
    VergeEngine() : vulkan(window.getReference(), window.getSize()), scene(vulkan.getContext(), 60.0f, window.getAspectRatio(), 0.1f, 1000.0f)
    {
        Input::init(window.getReference());
        Log::add('C', 000);
    }

    void run()
    {
        VE_STRUCT_VEHICLE_CREATE_INFO sCar = {};
        sCar.bodyMeshIndex = scene.loadFile("models/car.obj");
        sCar.wheelFLMeshIndex = scene.loadFile("models/wheel.obj");
        sCar.wheelFRMeshIndex = scene.loadFile("models/wheel.obj");
        sCar.wheelBLMeshIndex = scene.loadFile("models/wheel.obj");
        sCar.wheelBRMeshIndex = scene.loadFile("models/wheel.obj");
        sCar.wheelOffset = {2.0f, 0.4f, 1.8f};
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
        scene.addVehicle(sCar);

        VE_STRUCT_TRIGGER_TYPE_CREATE_INFO sTriggerType = {};
        sTriggerType.meshIndex = scene.loadFile("models/flag.obj");
        sTriggerType.hitboxShape = VE_SHAPE_SPHERE;
        sTriggerType.hitboxSize = 10.0f;
        sTriggerType.isAutoDestroy = true;
        scene.addTrigger(0, {2.0f, 0, 20.0f}, sTriggerType);
        scene.addTrigger(1, {2.0f, 0, 50.0f}, sTriggerType);

        scene.loadFile("models/floorBig.obj");

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
        scene.updateModel(5, glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0, 20.0f)));

        scene.tick(fps.getFrameTime());

        vulkan.drawFrame(scene.meshes, Camera::getProjectionMatrix(), Camera::getViewMatrix());
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