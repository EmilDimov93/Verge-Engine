// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer/Renderer.hpp"
#include "scene/Scene.hpp"

class VergeEngine
{
public:
    VergeEngine() : renderer({"Example", {1200, 600}}), scene({0.7f, 1.0f, 1.0f}) {}

    void run()
    {
        setupScene();

        while (renderer.tick(scene.getDrawData(player1), scene.getAudioData(player1)))
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
        carInfo.tireGrip = 2.5f;
        carInfo.camberRad = (PI / 180);
        carInfo.engineAudioFileName = "4k.wav";
        car1 = scene.addVehicle(carInfo, {{50.0f, 0, -40.0f}, {0, -PI / 4, 0}});
        
        carInfo.engineAudioFileName = "6k.wav";
        // VehicleHandle car2 = scene.addVehicle(carInfo, {{100.0f, 0, 0.0f}, {0, 5 * PI / 4, 0}});
        
        // Player
        PlayerKeybinds player1Keybinds{};
        player1Keybinds.throttle = VE_GAMEPAD_AXIS_RT;
        player1Keybinds.brake = VE_GAMEPAD_AXIS_LT;
        player1Keybinds.handbrake = VE_GAMEPAD_BTN_RB;
        player1Keybinds.steerLeft = VE_GAMEPAD_AXIS_LX_POS;
        player1Keybinds.steerRight = VE_GAMEPAD_AXIS_LX_NEG;
        player1Keybinds.shiftUp = VE_GAMEPAD_BTN_B;
        player1Keybinds.shiftDown = VE_GAMEPAD_BTN_X;

        player1 = scene.addPlayer(car1, player1Keybinds, {renderer.getAspectRatio()});

        // Prop
        scene.addProp(scene.loadFile("models/cow.obj"), {{-10.0f, 3.0f, 30.0f}});

        // Triggers
        VE_STRUCT_TRIGGER_TYPE_CREATE_INFO sTriggerType = {};
        sTriggerType.meshHandle = scene.loadFile("models/checkpoint.obj");
        sTriggerType.hitboxShape = VE_SHAPE_SPHERE;
        sTriggerType.hitboxSize = 10.0f;
        sTriggerType.isAutoDestroy = true;

        scene.addTrigger(sTriggerType, {{35.0f, 1.0f, 0.0f}, {0, PI / 2, 0}, {2.0f, 2.0f, 2.0f}});
        scene.addTrigger(sTriggerType, {{-35.0f, 1.0f, 60.0f}, {0, PI / 2, 0}, {2.0f, 2.0f, 2.0f}});

        // Ground
        uint32_t grassSurfaceTypeIndex = scene.addSurfaceType({0.6f, {0, 0.5f, 0}, {0, 0.05f, 0}, 0.1f});
        uint32_t asphaltSurfaceTypeIndex = scene.addSurfaceType({1.0f, {0.2f, 0.2f, 0.2f}, {0.01f, 0.0f, 0.0f}});

        uint32_t surfaceWidth = 1000;
        uint32_t surfaceHeight = 1000;

        std::vector<uint32_t> surfaceTypeMap;
        surfaceTypeMap.resize(surfaceWidth * surfaceHeight);

        std::vector<float> heightMap;
        heightMap.resize(surfaceWidth * surfaceHeight);

        for (uint32_t &surfaceType : surfaceTypeMap)
        {
            surfaceType = grassSurfaceTypeIndex;
        }

        const int roadHalfWidth = 10;
        const float curveStrength = 40.0f;
        const float curveFrequency = 0.05f;

        for (size_t i = 0; i < surfaceHeight; i++)
        {
            int centerX = static_cast<int>(surfaceWidth / 2 + std::cos(i * curveFrequency) * curveStrength);

            for (int j = centerX - roadHalfWidth; j <= centerX + roadHalfWidth; j++)
            {
                surfaceTypeMap[size_t(i) * surfaceWidth + j] = asphaltSurfaceTypeIndex;
                if (j >= 0 && j < surfaceWidth)
                {
                    surfaceTypeMap[size_t(i) * surfaceWidth + j] = asphaltSurfaceTypeIndex;
                    heightMap[size_t(i) * surfaceWidth + j] = 2.0f;
                }
            }
        }

        scene.addSurface({surfaceWidth, surfaceHeight}, surfaceTypeMap, heightMap);
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