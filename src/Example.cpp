// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client/Client.hpp"
#include "scene/Scene.hpp"

using namespace VE;

class VergeEngine
{
public:
    VergeEngine() : client({"Example", {1200, 1000}}) {}

    void run()
    {
        setupScene();
        setupClient();

        while (client.isOpen())
        {
            client.tick(scene.getDrawData(player1), scene.getAudioData(player1));

            scene.tick(client.getFrameTime(), {{player1, client.getVIS()}});
        }
    }

private:
    Client client;
    Scene scene;

    PlayerHandle player1;
    VehicleHandle car1;

    void setupClient()
    {
        client.setTargetFps(240);

        VehicleKeybinds keybinds(2);

        keybinds.throttle[0] = KEY_W;
        keybinds.brake[0] = KEY_S;
        keybinds.handbrake[0] = KEY_SPACE;
        keybinds.clutch[0] = KEY_E;
        keybinds.steerLeft[0] = KEY_A;
        keybinds.steerRight[0] = KEY_D;
        keybinds.shiftUp[0] = MOUSE_BTN_RIGHT;
        keybinds.shiftDown[0] = MOUSE_BTN_LEFT;
        keybinds.startEngine[0] = KEY_Q;
        keybinds.moveCameraLeft[0] = KEY_LEFT;
        keybinds.moveCameraRight[0] = KEY_RIGHT;
        keybinds.moveCameraUp[0] = KEY_UP;
        keybinds.moveCameraDown[0] = KEY_DOWN;

        keybinds.throttle[1] = CONTROLLER_AXIS_RT;
        keybinds.brake[1] = CONTROLLER_AXIS_LT;
        keybinds.handbrake[1] = CONTROLLER_BTN_RB;
        keybinds.clutch[1] = CONTROLLER_BTN_LB;
        keybinds.steerLeft[1] = CONTROLLER_AXIS_LX_NEGATIVE;
        keybinds.steerRight[1] = CONTROLLER_AXIS_LX_POSITIVE;
        keybinds.shiftUp[1] = CONTROLLER_BTN_X;
        keybinds.shiftDown[1] = CONTROLLER_BTN_Y;
        keybinds.startEngine[1] = CONTROLLER_BTN_A;
        keybinds.moveCameraLeft[1] = CONTROLLER_AXIS_RX_NEGATIVE;
        keybinds.moveCameraRight[1] = CONTROLLER_AXIS_RX_POSITIVE;
        keybinds.moveCameraUp[1] = CONTROLLER_AXIS_RY_POSITIVE;
        keybinds.moveCameraDown[1] = CONTROLLER_AXIS_RY_NEGATIVE;

        client.setVehicleKeybinds(keybinds);

        client.setSteerInputSmoothing(0.9f);

        PostEffects effects;
        effects.vignetteStrength = 0.8f;
        effects.vignetteRadius = 0.9f;
        effects.exposure = 1.f;
        effects.fxaa = true;
        effects.dithering = true;
        client.bindPostEffects(effects);

        client.ui.addWidgetInstance(client.ui.addWidget("models/button.obj"), {-0.85f, -0.85f}, 500.f, [](){ std::cout << "Button Clicked!" << std::endl; });
    }

    void setupScene()
    {
        scene.setBackgroundColor({0.7f, 1.f, 1.f, 1.f});

        // Vehicle
        VehicleCreateInfo carInfo = {};
        carInfo.bodyModelHandle = scene.addModel("models/X2/car.obj");
        carInfo.wheelModelHandle = scene.addModel("models/X2/wheel.obj");
        carInfo.wheelOffset = {1.05f, 0.5f, 1.8f};
        carInfo.peakTorqueNm = 480;
        carInfo.weightKg = 1540;
        carInfo.maxRpm = 7000;
        carInfo.idleRpm = 800;
        carInfo.transmissionType = TRANSMISSION_TYPE_AUTOMATIC;
        carInfo.gearRatios = {5.519f, 3.184f, 2.050f, 1.492f, 1.235f, 1.000f, 0.801f, 0.673f};
        carInfo.drivetrainEfficiency = 0.9f;
        carInfo.wheelRadiusM = 0.31f;
        carInfo.tireGrip = 1.f;
        carInfo.camberRad = (PI / 180);
        carInfo.drivetrainType = DRIVETRAIN_TYPE_RWD;
        carInfo.layeredEngineAudioFiles = {{"audio/4k.wav", 4000}};
        car1 = scene.addVehicle(carInfo);

        // Player
        player1 = scene.addPlayer(car1);

        // Prop
        scene.addProp(scene.addModel("models/cow.obj"), {{-10.f, 100.f, 30.f}}, 3.f, {1.f, 1.f, 0.9f, 1.f});

        // Triggers
        TriggerTypeCreateInfo sTriggerType = {};
        sTriggerType.modelHandle = scene.addModel("models/gdcheckpoint.glb");
        sTriggerType.hitboxShape = HITBOX_SHAPE_SPHERE;
        sTriggerType.hitboxSize = 10.f;
        sTriggerType.isAutoDestroy = true;

        scene.addTrigger(sTriggerType, {{-2.f, 3.f, -60.f}, {0, PI / 2, 0}, {2.f, 2.f, 2.f}}, [](){ std::cout << "Triggered 1" << std::endl; });
        scene.addTrigger(sTriggerType, {{2.f, 3.f, 60.f}, {0, PI / 2, 0}, {2.f, 2.f, 2.f}}, [](){ std::cout << "Triggered 2" << std::endl; });

        // Ground
        SurfaceTypeIndex grassSurfaceTypeIndex = scene.addSurfaceType({0.6f, {0.f, 0.4f, 0.f, 1.f}, 0.05f});
        SurfaceTypeIndex asphaltSurfaceTypeIndex = scene.addSurfaceType({1.f, {0.2f, 0.2f, 0.2f, 1.f}});
        SurfaceTypeIndex roadLineSurfaceTypeIndex = scene.addSurfaceType({1.f, {1.f, 1.f, 1.f, 1.f}});

        glm::uvec2 surfaceSize = {4000, 4000};

        std::vector<SurfaceTypeIndex> surfaceTypeMap;
        surfaceTypeMap.resize(surfaceSize.x * surfaceSize.y);

        std::vector<float> heightMap;
        heightMap.resize(surfaceSize.x * surfaceSize.y);

        for (uint32_t &surfaceType : surfaceTypeMap)
            surfaceType = grassSurfaceTypeIndex;

        const float curveStrength = 30.f;
        const float curveFrequency = 0.005f;
        const uint32_t roadHalfWidth = 30;
        for (size_t i = 0; i < surfaceSize.y; i++)
        {
            const int centerX = static_cast<int>(surfaceSize.x / 2 + std::cos(i * curveFrequency) * curveStrength);

            for (size_t j = centerX - roadHalfWidth; j <= centerX + roadHalfWidth; j++)
            {
                surfaceTypeMap[i * surfaceSize.x + j] = asphaltSurfaceTypeIndex;
                heightMap[i * surfaceSize.x + j] = 0.3f;
                if (j == centerX)
                {
                    surfaceTypeMap[i * surfaceSize.x + j] = roadLineSurfaceTypeIndex;
                }
            }
        }

        scene.addSurface(surfaceSize, surfaceTypeMap, heightMap, 0.2f);
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