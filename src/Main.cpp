// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer/Renderer.hpp"
#include "scene/Scene.hpp"

class VergeEngine
{
public:
    VergeEngine() : r1({"Window 1", {1200, 2000}}), r2({"Window 2", {1200, 2000}}), scene({0.7f, 1.0f, 1.0f}) {}

    void run()
    {
        setupScene();

        while (r1.tick(scene.getDrawData(p1h)) && r2.tick(scene.getDrawData(p2h)))
        {
            scene.tick(r1.getFrameTime());
        }
    }

private:
    Renderer r1;
    Renderer r2;
    Scene scene;

    PlayerId p1h;
    PlayerId p2h;

    void setupScene()
    {
        // Vehicle
        VE_STRUCT_VEHICLE_CREATE_INFO sCar = {};
        sCar.bodyMeshId = scene.loadFile("models/car.obj");
        sCar.wheelMeshId = scene.loadFile("models/wheel.obj");
        sCar.wheelOffset = {1.05f, 0.5f, 1.8f};
        sCar.power = 390;
        sCar.powerUnit = VE_POWER_UNIT_HORSEPOWER;
        sCar.weightKg = 1540;
        sCar.maxRpm = 7000;
        sCar.gearCount = 8;
        sCar.transmissionType = VE_TRANSMISSION_TYPE_AUTOMATIC;
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
        sCar.camberRad = (PI / 180);
        VehicleId car1 = scene.addVehicle(sCar, {{15.0f, 0, -100.0f}, {0, -PI / 4, 0}});

        VehicleId car2 = scene.addVehicle(sCar, {{15.0f, 0, -100.0f}, {0, -PI / 4, 0}});

        // Player
        PlayerKeybinds p1Keybinds{};
        p1Keybinds.throttle = VE_KEY_W;
        p1Keybinds.brake = VE_KEY_S;
        p1Keybinds.steerLeft = VE_KEY_A;
        p1Keybinds.steerRight = VE_KEY_D;

        PlayerKeybinds p2Keybinds{};
        p2Keybinds.throttle = VE_KEY_I;
        p2Keybinds.brake = VE_KEY_K;
        p2Keybinds.steerLeft = VE_KEY_J;
        p2Keybinds.steerRight = VE_KEY_L;

        p1h = scene.addPlayer(car1, p1Keybinds, {r1.getAspectRatio()});

        p2h = scene.addPlayer(car2, p2Keybinds, {r2.getAspectRatio()});

        // Prop
        scene.addProp(scene.loadFile("models/cow.obj"), {{-10.0f, 3.0f, 30.0f}});

        // Triggers
        VE_STRUCT_TRIGGER_TYPE_CREATE_INFO sTriggerType = {};
        sTriggerType.meshId = scene.loadFile("models/checkpoint.obj");
        sTriggerType.hitboxShape = VE_SHAPE_SPHERE;
        sTriggerType.hitboxSize = 10.0f;
        sTriggerType.isAutoDestroy = true;

        scene.addTrigger(0, sTriggerType, {{35.0f, 3.0f, 0.0f}, {0, PI / 2, 0}, {2.0f, 2.0f, 2.0f}});
        scene.addTrigger(1, sTriggerType, {{-35.0f, 3.0f, 60.0f}, {0, PI / 2, 0}, {2.0f, 2.0f, 2.0f}});

        // Ground
        uint32_t grassSurfaceIndex = scene.addSurface({0.2f, {0, 0.6f, 0}, {0, 0.05f, 0}, 0.2f});
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