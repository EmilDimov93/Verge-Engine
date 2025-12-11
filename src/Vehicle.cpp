// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#include <chrono>

const float gearRatios[8] = {5.519f, 3.184f, 2.050f, 1.492f, 1.235f, 1.000f, 0.801f, 0.673f};
const float finalDriveRatio = 3.2f;
const float drivetrainEfficiency = 0.9f;
const float wheelRadius = 0.31f;
const float idleRpm = 800.0f;
const float dragAccel = 0.5f;

void Vehicle::updateRmp()
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - lastTime;
    float deltaTime = elapsed.count();
    lastTime = currentTime;

    if (isGasDown)
    {
        if (rpm < 1.0f)
            rpm = idleRpm;
        float torqueCurveFactor = 0.85f + 0.15f * (1.0f - rpm / maxRpm);
        float torque = horsePower * 7127 / rpm * torqueCurveFactor;

        float wheelTorque = torque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;
        float wheelForce = wheelTorque / wheelRadius;

        float acceleration = wheelForce / weight;

        speed += acceleration * deltaTime;

        float wheelRpm = (speed / wheelRadius) * (60.0f / (2.0f * 3.1415926f));
        rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;

        if (rpm < idleRpm)
            rpm = idleRpm;
    }
    else
    {
        float rpmDropRate = 200.0f;
        rpm -= rpmDropRate * deltaTime;
        if (rpm < idleRpm)
            rpm = idleRpm;

        speed -= dragAccel * deltaTime;
        if (speed < 0.0f)
            speed = 0.0f;
    }

    if (rpm >= maxRpm && gear < gearCount)
    {
        float wheelRpm = rpm / (gearRatios[gear - 1] * finalDriveRatio);
        gear++;
        rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;
    }

    std::cout << "Speed: " << speed * 3.6f << " km/h, RPM: " << rpm << " , Gear: " << gear << std::endl;
}