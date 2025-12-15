// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"
#include "Log.hpp"

#define PI 3.1415926f

#define TORQUE_CONVERSION_CONSTANT 5252
#define GRAVITY_CONSTANT 9.81f

#define AIR_DENSITY 1.225f

#define BASELINE_TORQUE_FACTOR  0.9f
#define SURFACE_ROLLING_COEFFICIENT 0.015f

void Vehicle::init(const VE_STRUCT_VEHICLE_CREATE_INFO &info)
{
    if (info.pBody)
    {
        body = *(info.pBody);
    }
    else
    {
        Log::add('A', 100);
    }
    if (info.pTire)
    {
        tire = *(info.pTire);
    }
    else
    {
        Log::add('A', 101);
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
            break;
        default:
            Log::add('A', 102);
            powerKw = info.power;
            break;
        }
    }
    else
    {
        Log::add('A', 103);
        powerKw = 100;
    }

    if (info.weightKg > 0)
    {
        weightKg = info.weightKg;
    }
    else
    {
        Log::add('A', 104);
        weightKg = 1200.f;
    }

    if (info.gearCount > 0)
    {
        gearCount = info.gearCount;
    }
    else
    {
        Log::add('A', 105);
        gearCount = 5;
    }

    if (info.idleRpm > 0)
    {
        idleRpm = info.idleRpm;
    }
    else
    {
        Log::add('A', 106);
        idleRpm = 800;
    }

    if (info.maxRpm > idleRpm)
    {
        maxRpm = info.maxRpm;
    }
    else
    {
        Log::add('A', 107);
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
        // error if pGearRatios size is lower than gearCount
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
        Log::add('A', 108);
        drivetrainEfficiency = 1.0f;
    }

    if (info.wheelRadiusM > 0)
    {
        wheelRadiusM = info.wheelRadiusM;
    }
    else
    {
        Log::add('A', 109);
        wheelRadiusM = 0.3f;
    }

    if (info.dragCoeff >= 0)
    {
        dragCoeff = info.dragCoeff;
    }
    else
    {
        Log::add('A', 110);
        dragCoeff = 0.31f;
    }

    if (info.frontalAreaM2 > 0)
    {
        frontalAreaM2 = info.frontalAreaM2;
    }
    else
    {
        Log::add('A', 111);
        frontalAreaM2 = 0.0009f * info.weightKg + 0.5f;
    }

    if (info.maxSteeringAngleRad > 0 && info.maxSteeringAngleRad <= 0.9f)
    { // Hardcoded limit
        maxSteeringAngleRad = info.maxSteeringAngleRad;
    }
    else if (info.maxSteeringAngleRad >= -0.9f && info.maxSteeringAngleRad <= 0.9f)
    {
        Log::add('A', 112);
        maxSteeringAngleRad = -info.maxSteeringAngleRad;
    }
    else
    {
        Log::add('A', 113);
        maxSteeringAngleRad = 0.55f;
    }

    if (info.camber > -(PI / 2) && info.camber < PI / 2)
    {
        camber = info.camber;
    }
    else
    {
        Log::add('A', 114);
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
    float torqueCurveFactor = BASELINE_TORQUE_FACTOR + (1.0f - BASELINE_TORQUE_FACTOR) * (1.0f - rpm / maxRpm);
    float torque = (powerKw * 1000) / (rpm * 2.0f * PI / 60.0f) * torqueCurveFactor;

    float wheelTorque = torque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;
    float wheelForce = wheelTorque / wheelRadiusM;

    float dragForce = (AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps) / 2;
    float acceleration = (wheelForce - dragForce) / weightKg;

    speedMps += acceleration * deltaTime;

    float wheelRpm = (speedMps / wheelRadiusM) * (60.0f / (2.0f * PI));
    rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;

    if (rpm >= maxRpm)
        rpm = maxRpm;
    else if (rpm < idleRpm)
        rpm = idleRpm;

    if (isAutomatic && gear < gearCount && rpm >= maxRpm)
    {
        float wheelRpm = rpm / (gearRatios[gear - 1] * finalDriveRatio);
        gear++;
        rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;
    }
    else
    {
        //rpm = maxRpm;
    }
}

void Vehicle::idle(ve_time deltaTime)
{
    float rollingResistance = SURFACE_ROLLING_COEFFICIENT * weightKg * GRAVITY_CONSTANT;
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
    steeringAngleRad = maxSteeringAngleRad;
}

void Vehicle::turnRight()
{
    steeringAngleRad = -maxSteeringAngleRad;
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

    std::cout << "Speed: " << speedMps * 3.6f << " km/h, RPM: " << std::round(rpm) << " , Gear: " << gear << std::endl;
}