// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define PI 3.1415926f
#define AIR_DENSITY 1.225f

void Vehicle::accelerate(ve_time deltaTime)
{
    if (rpm >= maxRpm)
    {
        if (isAutomatic && gear < gearCount)
        {
            float wheelRpm = rpm / (gearRatios[gear - 1] * finalDriveRatio);
            gear++;
            rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;
        }
        else
        {
            rpm = maxRpm;
        }
    }
    else if (rpm < idleRpm)
        rpm = idleRpm;

    float torqueCurveFactor = 0.85f + 0.15f * (1.0f - rpm / maxRpm);
    float torque = horsePower * 5252 / rpm * torqueCurveFactor;

    float wheelTorque = torque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;
    float wheelForce = wheelTorque / wheelRadius;

    float dragForce = 0.5f * AIR_DENSITY * dragCoeff * frontalArea * speed * speed;
    float acceleration = (wheelForce - dragForce) / weight;

    speed += acceleration * deltaTime;

    float wheelRpm = (speed / wheelRadius) * (60.0f / (2.0f * PI));
    rpm = rpm < maxRpm ? wheelRpm * gearRatios[gear - 1] * finalDriveRatio : maxRpm;

    if (rpm < idleRpm)
        rpm = idleRpm;
}

void Vehicle::idle(ve_time deltaTime)
{
    float rollingResistance = 0.015f * weight * 9.81f;
    float dragForce = 0.5f * AIR_DENSITY * dragCoeff * frontalArea * speed * speed;
    float netDecel = (dragForce + rollingResistance) / weight;
    float wheelAngularDecel = netDecel / wheelRadius;
    float rpmDropRate = wheelAngularDecel * (60.0f / (2.0f * PI)) * gearRatios[gear - 1] * finalDriveRatio;
    rpm -= rpmDropRate * deltaTime;
    if (rpm < idleRpm)
        rpm = idleRpm;

    speed -= dragAccel * deltaTime;
    if (speed < 0.0f)
        speed = 0.0f;
}

void Vehicle::brake(ve_time deltaTime)
{
}

void Vehicle::update(ve_time deltaTime)
{
    if (Input::isDown(accelerateKey))
    {
        accelerate(deltaTime);
    }
    else
    {
        idle(deltaTime);
    }

    std::cout << "Speed: " << speed * 3.6f << " km/h, RPM: " << rpm << " , Gear: " << gear << std::endl;
}