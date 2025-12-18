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

class VergeEngine
{
public:
    VergeEngine() : vulkan(window.getWindowReference(), window.getWindowSize())
    {
        Input::init(window.getWindowReference());
        Log::add('C', 000);
        Camera::init(60.0f, window.getAspectRatio(), 0.1f, 1000.0f);
    }

    void run()
    {
        VE_STRUCT_VEHICLE_CREATE_INFO sCar = {};
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
        car.init(sCar);

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

    Vehicle car;

    void tick()
    {
        float dt = fps.getFrameTime();

        car.update(dt);

        glm::mat4 firstModel(1.0f);
        glm::mat4 secondModel(1.0f);

        firstModel = glm::translate(firstModel, glm::vec3(2.0f, 0.0f, -4.0f));
        firstModel = glm::rotate(firstModel, car.getSteeringAngleRad(), glm::vec3(0.0, 0.0f, 1.0f));

        static float rotation = 0.0f;

        rotation += car.getSpeedMps() / car.getWheelRadius() * dt;

        secondModel = glm::translate(secondModel, glm::vec3(-2.0f, 0.0f, -5.0f));
        secondModel = glm::rotate(secondModel, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 carModel(1.0f);

        static float z = 100.0f;

        z -= car.getSpeedMps() * dt;

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

        float rad = glm::radians(cameraRot);

        float camX = 0 + sin(rad) * distance;
        float camZ = z + cos(rad) * distance;
        float camY = 0 + height;

        Camera::move({camX, camY, camZ});

        glm::vec3 dir = glm::normalize(glm::vec3(0, 0, z) - glm::vec3(camX, camY, camZ));
        float pitch = glm::degrees(asin(dir.y));
        float yaw = glm::degrees(atan2(dir.z, dir.x));
        Camera::rotate({pitch, yaw, 0.0f});

        glm::mat4 tireFL(1.0f);
        glm::mat4 tireFR(1.0f);
        glm::mat4 tireBL(1.0f);
        glm::mat4 tireBR(1.0f);

        Position2 tireOffset = {2.5f, 2.0f};

        tireFL = glm::translate(tireFL, glm::vec3(tireOffset.x / 2, 0.0f, z - tireOffset.y));
        tireFR = glm::translate(tireFR, glm::vec3(-tireOffset.x / 2, 0.0f, z - tireOffset.y));
        tireBL = glm::translate(tireBL, glm::vec3(tireOffset.x / 2, 0.0f, z + tireOffset.y));
        tireBR = glm::translate(tireBR, glm::vec3(-tireOffset.x / 2, 0.0f, z + tireOffset.y));

        tireFL = glm::rotate(tireFL, car.getSteeringAngleRad(), glm::vec3(0.0f, 1.0f, 0.0f));
        tireFR = glm::rotate(tireFR, car.getSteeringAngleRad(), glm::vec3(0.0f, 1.0f, 0.0f));

        tireFL = glm::rotate(tireFL, rotation, glm::vec3(-1.0f, 0.0f, 0.0f));
        tireFR = glm::rotate(tireFR, rotation, glm::vec3(-1.0f, 0.0f, 0.0f));
        tireBL = glm::rotate(tireBL, rotation, glm::vec3(-1.0f, 0.0f, 0.0f));
        tireBR = glm::rotate(tireBR, rotation, glm::vec3(-1.0f, 0.0f, 0.0f));

        vulkan.updateModel(0, firstModel);
        vulkan.updateModel(1, secondModel);

        vulkan.updateModel(2, carModel);
        vulkan.updateModel(3, tireFL);
        vulkan.updateModel(4, tireFR);
        vulkan.updateModel(5, tireBL);
        vulkan.updateModel(6, tireBR);

        Camera::update();

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