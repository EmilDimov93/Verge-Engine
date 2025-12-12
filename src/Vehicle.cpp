// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define PI 3.1415926f

#define TORQUE_CONVERSION_CONSTANT 5252

#define AIR_DENSITY 1.225f

void Vehicle::init(const VE_STRUCT_VEHICLE_CREATE_INFO &info)
{
    if (info.pBody)
    {
        body = *(info.pBody);
    }
    else
    {
        // warning
    }
    if (info.pTire)
    {
        tire = *(info.pTire);
    }
    else
    {
        // warning
    }

    if (info.powerUnit == VE_POWER_UNIT_KILOWATTS)
    {
        powerKw = info.power;
    }
    else if (info.powerUnit == VE_POWER_UNIT_HORSEPOWER)
    {
        powerKw = 0.7457f * info.power;
    }
    else
    {
        // warning
        powerKw = info.power;
    }
    weightKg = info.weightKg;
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
    drivetrainEfficiency = info.drivetrainEfficiency;
    wheelRadiusM = info.wheelRadiusM;
    idleRpm = info.idleRpm;
    dragCoeff = info.dragCoeff;
    frontalAreaM2 = info.frontalAreaM2;
    maxSteeringAngleRad = info.maxSteeringAngleRad;

    ////////////////

    steeringAngleRad = 0;
    speedMps = 0;
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
    float torque = (powerKw * 1000) / (rpm * 2.0f * PI / 60.0f) * torqueCurveFactor;

    float wheelTorque = torque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;
    float wheelForce = wheelTorque / wheelRadiusM;

    float dragForce = 0.5f * AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps;
    float acceleration = (wheelForce - dragForce) / weightKg;

    speedMps += acceleration * deltaTime;

    float wheelRpm = (speedMps / wheelRadiusM) * (60.0f / (2.0f * PI));
    rpm = rpm < maxRpm ? wheelRpm * gearRatios[gear - 1] * finalDriveRatio : maxRpm;

    if (rpm < idleRpm)
        rpm = idleRpm;
}

void Vehicle::idle(ve_time deltaTime)
{
    float rollingResistance = 0.015f * weightKg * 9.81f;
    float dragForce = 0.5f * AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps;
    float netDecel = (dragForce + rollingResistance) / weightKg;

    speedMps -= netDecel * deltaTime;
    if (speedMps < 0.0f)
        speedMps = 0.0f;

    float wheelAngularDecel = netDecel / wheelRadiusM;
    float rpmDropRate = wheelAngularDecel * (60.0f / (2.0f * PI)) * gearRatios[gear - 1] * finalDriveRatio;
    rpm -= rpmDropRate * deltaTime;
    if (rpm < idleRpm)
        rpm = idleRpm;
}

void Vehicle::brake(ve_time deltaTime)
{
}

void Vehicle::turnLeft()
{
    steeringAngleRad = -maxSteeringAngleRad;
}

void Vehicle::turnRight()
{
    steeringAngleRad = maxSteeringAngleRad;
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

    steeringAngleRad = 0;
    if(Input::isDown(turnLeftKey) && Input::isUp(turnRightKey)){
        turnLeft();
    }
    else if(Input::isDown(turnRightKey) && Input::isUp(turnLeftKey)){
        turnRight();
    }

    //std::cout << "Speed: " << speedMps * 3.6f << " km/h, RPM: " << rpm << " , Gear: " << gear << std::endl;
    std::cout << steeringAngleRad << std::endl;
}