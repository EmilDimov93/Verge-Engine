// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer/Renderer.hpp"
#include "scene/Scene.hpp"

class VergeEngine
{
public:
    VergeEngine() : renderer({"Example", {1200, 600}}) {}

    void run()
    {
        setupScene();

        setupRenderer();

        while (renderer.isOpen())
        {
            renderer.tick(scene.getDrawData(player1), scene.getAudioData(player1));

            scene.tick(renderer.getFrameTime(), {{player1, renderer.getVIS()}});
        }
    }

private:
    Renderer renderer;
    Scene scene;

    PlayerHandle player1;
    VehicleHandle car1;

    void setupRenderer()
    {
        renderer.setTargetFps(240);

        VehicleKeybinds keybinds{};

        keybinds.throttle[0] = VE_KEY_W;
        keybinds.brake[0] = VE_KEY_S;
        keybinds.handbrake[0] = VE_KEY_SPACE;
        keybinds.clutch[0] = VE_KEY_E;
        keybinds.steerLeft[0] = VE_KEY_A;
        keybinds.steerRight[0] = VE_KEY_D;
        keybinds.shiftUp[0] = VE_MOUSE_BTN_RIGHT;
        keybinds.shiftDown[0] = VE_MOUSE_BTN_LEFT;
        keybinds.startEngine[0] = VE_KEY_Q;
        keybinds.moveCameraLeft[0] = VE_KEY_LEFT;
        keybinds.moveCameraRight[0] = VE_KEY_RIGHT;
        keybinds.moveCameraUp[0] = VE_KEY_UP;
        keybinds.moveCameraDown[0] = VE_KEY_DOWN;

        keybinds.throttle[1] = VE_CONTROLLER_AXIS_RT;
        keybinds.brake[1] = VE_CONTROLLER_AXIS_LT;
        keybinds.handbrake[1] = VE_CONTROLLER_BTN_RB;
        keybinds.clutch[1] = VE_CONTROLLER_BTN_LB;
        keybinds.steerLeft[1] = VE_CONTROLLER_AXIS_LX_NEGATIVE;
        keybinds.steerRight[1] = VE_CONTROLLER_AXIS_LX_POSITIVE;
        keybinds.shiftUp[1] = VE_CONTROLLER_BTN_X;
        keybinds.shiftDown[1] = VE_CONTROLLER_BTN_Y;
        keybinds.startEngine[1] = VE_CONTROLLER_BTN_A;
        keybinds.moveCameraLeft[1] = VE_CONTROLLER_AXIS_RX_NEGATIVE;
        keybinds.moveCameraRight[1] = VE_CONTROLLER_AXIS_RX_POSITIVE;
        keybinds.moveCameraUp[1] = VE_CONTROLLER_AXIS_RY_POSITIVE;
        keybinds.moveCameraDown[1] = VE_CONTROLLER_AXIS_RY_NEGATIVE;

        renderer.setVehicleKeybinds(keybinds);
    }

    void setupScene()
    {
        scene.setBackgroundColor({0.7f, 1.0f, 1.0f});

        // Vehicle
        VE_STRUCT_VEHICLE_CREATE_INFO carInfo = {};
        carInfo.bodyMeshHandle = scene.loadFile("models/car.obj");
        carInfo.wheelMeshHandle = scene.loadFile("models/wheel.obj");
        carInfo.wheelOffset = {1.05f, 0.5f, 1.8f};
        carInfo.peakTorqueNm = 480;
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
        carInfo.brakingForceN = 14700;
        carInfo.tireGrip = 1.0f;
        carInfo.camberRad = (PI / 180);
        carInfo.drivetrainType = VE_DRIVETRAIN_TYPE_FWD;
        carInfo.engineAudioFileName = "4k.wav";
        // carInfo.layeredEngineAudioFiles = {{"4k.wav", 1000}, {"4k.wav", 2000}, {"4k.wav", 3000}, {"4k.wav", 4000}, {"4k.wav", 5000}, {"4k.wav", 6000}, {"4k.wav", 7000}};
        car1 = scene.addVehicle(carInfo, {{50.0f, 0, -40.0f}, {0, -PI / 4, 0}});

        // Player
        player1 = scene.addPlayer(car1, {renderer.getAspectRatio()});

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
        SurfaceTypeIndex grassSurfaceTypeIndex = scene.addSurfaceType({0.6f, {0, 0.5f, 0}, {0, 0.05f, 0}, 0.1f});
        SurfaceTypeIndex asphaltSurfaceTypeIndex = scene.addSurfaceType({1.0f, {0.2f, 0.2f, 0.2f}, {0.01f, 0.0f, 0.0f}});
        SurfaceTypeIndex roadLineSurfaceTypeIndex = scene.addSurfaceType({1.0f, {1.0f, 1.0f, 1.0f}, {}});

        Size2 surfaceSize = {1000, 1000};

        std::vector<SurfaceTypeIndex> surfaceTypeMap;
        surfaceTypeMap.resize(surfaceSize.w * surfaceSize.h);

        std::vector<float> heightMap;
        heightMap.resize(surfaceSize.w * surfaceSize.h);

        for (uint32_t &surfaceType : surfaceTypeMap)
            surfaceType = grassSurfaceTypeIndex;

        const float curveStrength = 5.0f;
        const float curveFrequency = 0.05f;
        const int roadHalfWidth = 10;
        for (size_t i = 0; i < surfaceSize.h; i++)
        {
            const int centerX = static_cast<int>(surfaceSize.w / 2 + std::cos(i * curveFrequency) * curveStrength);

            for (size_t j = centerX - roadHalfWidth; j <= centerX + roadHalfWidth; j++)
            {
                surfaceTypeMap[i * surfaceSize.w + j] = asphaltSurfaceTypeIndex;
                heightMap[i * surfaceSize.w + j] = 0.3f;
                if (j == centerX)
                {
                    surfaceTypeMap[i * surfaceSize.w + j] = roadLineSurfaceTypeIndex;
                }
            }
        }

        scene.addSurface(surfaceSize, surfaceTypeMap, heightMap);
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