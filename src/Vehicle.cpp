// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define PI 3.1415926f
#define AIR_DENSITY 1.225f

void Vehicle::init(const VE_STRUCT_VEHICLE_CREATE_INFO &info)
{
    //body = *(info.pBody);
    //tire = *(info.pTire);

    if(!info.pBody){
        // warning
    }
    if(!info.pTire){
        // warning
    }

    horsePower = info.horsePower;
    weight = info.weight;
    gearCount = info.gearCount;
    maxRpm = info.maxRpm;
    isAutomatic = info.isAutomatic;
    brakingForce = info.brakingForce;

    accelerateKey = info.accelerateKey;
    brakeKey = info.brakeKey;
    turnLeftKey = info.turnLeftKey;
    turnRightKey = info.turnRightKey;

    if (info.pGearRatios)
    {
        gearRatios.assign(info.pGearRatios, info.pGearRatios + gearCount);
    }
    else
    {
        const float defaultTopRatio = 1.0f;
        const float defaultFirstRatio = 5.0f;
        gearRatios.resize(gearCount);
        for (size_t i = 0; i < gearCount; ++i)
        {
            gearRatios[i] = defaultTopRatio * std::pow(defaultFirstRatio / defaultTopRatio, float(gearCount - 1 - i) / float(gearCount - 1));
        }
    }

    finalDriveRatio = info.finalDriveRatio;
    drivetrainEfficiency = info.driveTrainEfficiency;
    wheelRadius = info.wheelRadius;
    idleRpm = info.idleRpm;
    dragAccel = info.dragAccel;
    dragCoeff = info.dragCoeff;
    frontalArea = info.frontalArea;

    ////////////////

    tireRotation = 0;
    speed = 0;
    gear = 1;
    rpm = 0;
}

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

void Vehicle::turnLeft()
{
}

void Vehicle::turnRight()
{
}

void Vehicle::update(ve_time deltaTime)
{
    if (Input::isDown(accelerateKey))
    {
        accelerate(deltaTime);
    }
    else if (Input::isDown(brakeKey))
    {
        brake(deltaTime);
    }
    else
    {
        idle(deltaTime);
    }

    if (Input::isDown(turnLeftKey))
    {
        turnLeft();
    }
    if (Input::isDown(turnRightKey))
    {
        turnRight();
    }

    std::cout << "Speed: " << speed * 3.6f << " km/h, RPM: " << rpm << " , Gear: " << gear << std::endl;
}