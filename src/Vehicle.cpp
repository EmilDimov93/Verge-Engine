// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define PI 3.1415926f

#define TORQUE_CONVERSION_CONSTANT 5252
#define GRAVITY_CONSTANT 9.81f

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

    if (info.power > 0)
    {
        switch (info.powerUnit)
        {
        case VE_POWER_UNIT_KILOWATTS:
            powerKw = info.power;
            break;
        case VE_POWER_UNIT_HORSEPOWER:
            powerKw = 0.7457f * info.power;
        default:
            // warning
            powerKw = info.power;
            break;
        }
    }
    else
    {
        // warning
        powerKw = 100;
    }

    if (info.weightKg > 0)
    {
        weightKg = info.weightKg;
    }
    else
    {
        // warning
        weightKg = 1200.f;
    }

    if (gearCount > 0)
    {
        gearCount = info.gearCount;
    }
    else
    {
        // warning
        gearCount = 5;
    }

    if (idleRpm > 0)
    {
        idleRpm = info.idleRpm;
    }
    else
    {
        // warning
        idleRpm = 800;
    }

    if (info.maxRpm > idleRpm)
    {
        maxRpm = info.maxRpm;
    }
    else
    {
        // warning
        maxRpm = 6000;
    }

    isAutomatic = info.isAutomatic;

    if (info.brakingForce >= 0 && info.brakingForce <= 1.0f)
    {
        brakingForce = info.brakingForce;
    }
    else
    {
        brakingForce = 1.0f;
    }

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
        if (gearCount == 1)
        {
            gearRatios[0] = 1.0f;
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
    }

    finalDriveRatio = info.finalDriveRatio;

    if (info.drivetrainEfficiency >= 0 && info.drivetrainEfficiency <= 1.0f)
    {
        drivetrainEfficiency = info.drivetrainEfficiency;
    }
    else
    {
        drivetrainEfficiency = 1.0f;
    }

    if (info.wheelRadiusM > 0)
    {
        wheelRadiusM = info.wheelRadiusM;
    }
    else
    {
        // warning
        wheelRadiusM = 0.3f;
    }

    if (info.dragCoeff >= 0)
    {
        dragCoeff = info.dragCoeff;
    }
    else
    {
        // warning
        dragCoeff = 0.31f;
    }

    if (info.frontalAreaM2 > 0)
    {
        frontalAreaM2 = info.frontalAreaM2;
    }
    else
    {
        // warning
        frontalAreaM2 = 0.0009f * info.weightKg + 0.5f;
    }

    if (info.maxSteeringAngleRad > 0 && info.maxSteeringAngleRad <= 0.9f)
    { // Hardcoded limit
        maxSteeringAngleRad = info.maxSteeringAngleRad;
    }
    else if (info.maxSteeringAngleRad <= 0.9f)
    {
        // warning
        maxSteeringAngleRad = -info.maxSteeringAngleRad;
    }
    else
    {
        // warning
        maxSteeringAngleRad = 0.55f;
    }

    if (camber > -(PI / 2) && camber < PI / 2)
    {
        camber = info.camber;
    }
    else
    {
        // warning
        camber = 0;
    }

    ////////////////

    steeringAngleRad = 0;
    speedMps = 0;
    gear = 1;
    rpm = 0;
    clutchLevel = 0.0f;
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

    constexpr float BASELINE_TORQUE_FACTOR = 0.9f;
    float torqueCurveFactor = BASELINE_TORQUE_FACTOR + (1.0f - BASELINE_TORQUE_FACTOR) * (1.0f - rpm / maxRpm);
    float torque = (powerKw * 1000) / (rpm * 2.0f * PI / 60.0f) * torqueCurveFactor;

    float wheelTorque = torque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;
    float wheelForce = wheelTorque / wheelRadiusM;

    float dragForce = (AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps) / 2;
    float acceleration = (wheelForce - dragForce) / weightKg;

    speedMps += acceleration * deltaTime;

    float wheelRpm = (speedMps / wheelRadiusM) * (60.0f / (2.0f * PI));
    rpm = rpm < maxRpm ? wheelRpm * gearRatios[gear - 1] * finalDriveRatio : maxRpm;

    if (rpm < idleRpm)
        rpm = idleRpm;
}

void Vehicle::idle(ve_time deltaTime)
{
    float rollingResistance = 0.015f * weightKg * GRAVITY_CONSTANT;
    float dragForce = (AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps) / 2;
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
    if (Input::isDown(turnLeftKey) && Input::isUp(turnRightKey))
    {
        turnLeft();
    }
    else if (Input::isDown(turnRightKey) && Input::isUp(turnLeftKey))
    {
        turnRight();
    }

    std::cout << "Speed: " << speedMps * 3.6f << " km/h, RPM: " << rpm << " , Gear: " << gear << std::endl;
}