// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer/Renderer.hpp"
#include "scene/Scene.hpp"

class VergeEngine
{
public:
    VergeEngine() : renderer({"Example", {1200, 2000}}), scene({0.7f, 1.0f, 1.0f}) {}

    void run()
    {
        setupScene();

        while (renderer.tick(scene.getDrawData(player1)))
        {
            scene.tick(renderer.getFrameTime());
        }
    }

private:
    Renderer renderer;
    Scene scene;

    PlayerHandle player1;
    VehicleHandle car1;

    void setupScene()
    {
        // Vehicle
        VE_STRUCT_VEHICLE_CREATE_INFO carInfo = {};
        carInfo.bodyMeshHandle = scene.loadFile("models/car.obj");
        carInfo.wheelMeshHandle = scene.loadFile("models/wheel.obj");
        carInfo.wheelOffset = {1.05f, 0.5f, 1.8f};
        carInfo.power = 390;
        carInfo.powerUnit = VE_POWER_UNIT_HORSEPOWER;
        carInfo.weightKg = 1540;
        carInfo.maxRpm = 7000;
        carInfo.idleRpm = 800;
        carInfo.gearCount = 8;
        carInfo.transmissionType = VE_TRANSMISSION_TYPE_AUTOMATIC;
        float ratios[8] = {5.519f, 3.184f, 2.050f, 1.492f, 1.235f, 1.000f, 0.801f, 0.673f};
        carInfo.pGearRatios = ratios;
        carInfo.finalDriveRatio = 3.2f;
        carInfo.drivetrainEfficiency = 0.9f;
        carInfo.wheelRadiusM = 0.31f;
        carInfo.dragCoeff = 0.31f;
        carInfo.frontalAreaM2 = 2.3f;
        carInfo.brakingForce = 14700;
        carInfo.tireGrip = 1.5f;
        carInfo.camberRad = (PI / 180);
        car1 = scene.addVehicle(carInfo, {{17.0f, 3.0f, -100.0f}, {0, -PI / 4, 0}});

        // Player
        PlayerKeybinds player1Keybinds{};
        player1Keybinds.throttle = VE_KEY_W;
        player1Keybinds.brake = VE_KEY_S;
        player1Keybinds.steerLeft = VE_KEY_A;
        player1Keybinds.steerRight = VE_KEY_D;

        player1 = scene.addPlayer(car1, player1Keybinds, {renderer.getAspectRatio()});

        // Prop
        scene.addProp(scene.loadFile("models/cow.obj"), {{-10.0f, 3.0f, 30.0f}});

        // Triggers
        VE_STRUCT_TRIGGER_TYPE_CREATE_INFO sTriggerType = {};
        sTriggerType.meshHandle = scene.loadFile("models/checkpoint.obj");
        sTriggerType.hitboxShape = VE_SHAPE_SPHERE;
        sTriggerType.hitboxSize = 10.0f;
        sTriggerType.isAutoDestroy = true;

        scene.addTrigger(sTriggerType, {{35.0f, 3.0f, 0.0f}, {0, PI / 2, 0}, {2.0f, 2.0f, 2.0f}});
        scene.addTrigger(sTriggerType, {{-35.0f, 3.0f, 60.0f}, {0, PI / 2, 0}, {2.0f, 2.0f, 2.0f}});

        // Ground
        uint32_t grassSurfaceIndex = scene.addSurface({0.6f, {0, 0.5f, 0}, {0, 0.05f, 0}, 0.2f});
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