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
    VergeEngine() : vulkan(window.getWindowReference(), window.getWindowSize()), scene(vulkan.getContext())
    {
        Input::init(window.getWindowReference());
        Log::add('C', 000);
        Camera::init(60.0f, window.getAspectRatio(), 0.1f, 1000.0f);
    }

    void run()
    {
        VE_STRUCT_VEHICLE_CREATE_INFO sCar = {};
        sCar.bodyMeshIndex = 0;
        sCar.tireFLMeshIndex = 1;
        sCar.tireFRMeshIndex = 2;
        sCar.tireBLMeshIndex = 3;
        sCar.tireBRMeshIndex = 4;
        sCar.power = 190;
        sCar.powerUnit = VE_POWER_UNIT_HORSEPOWER;
        sCar.weightKg = 1540;
        sCar.maxRpm = 7000;
        sCar.gearCount = 8;
        sCar.transmissionType = VE_TRANSMISSION_TYPE_AUTOMATIC;
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
        scene.addVehicle(sCar);

        scene.loadFile("models/car.obj", {0, 0, 0});

        scene.loadFile("models/wheel.obj", {0, 0, 1.0f});
        scene.loadFile("models/wheel.obj", {0, 0, 1.0f});
        scene.loadFile("models/wheel.obj", {0, 0, 1.0f});
        scene.loadFile("models/wheel.obj", {0, 0, 1.0f});

        scene.loadFile("models/flag.obj", {0, 1.0f, 0});
        scene.loadFile("models/flag.obj", {0, 1.0f, 0});

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
        ve_time dt = fps.getFrameTime();

        static float rotation = 0;

        rotation += scene.vehicles[0].getSpeedMps() / scene.vehicles[0].getWheelRadius() * dt;

        glm::mat4 carModel(1.0f);

        static float z = 100.0f;

        z -= scene.vehicles[0].getSpeedMps() * dt;

        carModel = glm::translate(carModel, glm::vec3(0, 0, z));

        static float cameraRot = 0;
        if (Input::isDown(VE_KEY_LEFT))
        {
            cameraRot += dt * 90;
        }
        if (Input::isDown(VE_KEY_RIGHT))
        {
            cameraRot -= dt * 90;
        }

        float distance = 8.0f;
        float height = 3.0f;

        float camX = 0 + sin(glm::radians(cameraRot)) * distance;
        float camZ = z + cos(glm::radians(cameraRot)) * distance;
        float camY = 0 + height;

        Camera::move({camX, camY, camZ});

        glm::vec3 dir = glm::normalize(glm::vec3(0, 0, z) - glm::vec3(camX, camY, camZ));
        float pitch = glm::degrees(asin(dir.y));
        float yaw = glm::degrees(atan2(dir.z, dir.x));
        Camera::rotate({pitch, yaw, 0});

        glm::mat4 tireFL(1.0f), tireFR(1.0f), tireBL(1.0f), tireBR(1.0f);

        Position2 tireOffset = {2.5f, 2.0f};

        tireFL = glm::translate(tireFL, glm::vec3(tireOffset.x / 2, 0, z - tireOffset.y));
        tireFR = glm::translate(tireFR, glm::vec3(-tireOffset.x / 2, 0, z - tireOffset.y));
        tireBL = glm::translate(tireBL, glm::vec3(tireOffset.x / 2, 0, z + tireOffset.y));
        tireBR = glm::translate(tireBR, glm::vec3(-tireOffset.x / 2, 0, z + tireOffset.y));

        tireFL = glm::rotate(tireFL, scene.vehicles[0].getSteeringAngleRad(), glm::vec3(0, 1.0f, 0));
        tireFR = glm::rotate(tireFR, scene.vehicles[0].getSteeringAngleRad(), glm::vec3(0, 1.0f, 0));

        tireFL = glm::rotate(tireFL, rotation, glm::vec3(-1.0f, 0, 0));
        tireFR = glm::rotate(tireFR, rotation, glm::vec3(-1.0f, 0, 0));
        tireBL = glm::rotate(tireBL, rotation, glm::vec3(-1.0f, 0, 0));
        tireBR = glm::rotate(tireBR, rotation, glm::vec3(-1.0f, 0, 0));

        scene.vehicles[0].bodyMat = carModel;
        scene.vehicles[0].tireFLMat = tireFL;
        scene.vehicles[0].tireFRMat = tireFR;
        scene.vehicles[0].tireBLMat = tireBL;
        scene.vehicles[0].tireBRMat = tireBR;

        glm::mat4 flag1Model(1.0f), flag2Model(1.0f);

        flag1Model = glm::translate(flag1Model, glm::vec3(2.0f, 0, 20.0f));
        flag2Model = glm::translate(flag2Model, glm::vec3(-2.0f, 0, 20.0f));

        scene.updateModel(5, flag1Model);
        scene.updateModel(6, flag2Model);

        scene.tick(dt);

        Camera::update();

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