// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define TORQUE_CONVERSION_CONSTANT 5252
#define GRAVITY_CONSTANT 9.81f

#define AIR_DENSITY 1.225f

#define BASELINE_TORQUE_FACTOR 0.9f
#define SURFACE_ROLLING_COEFFICIENT 0.015f

void Vehicle::accelerate()
{
    float torqueCurveFactor = BASELINE_TORQUE_FACTOR + (1.0f - BASELINE_TORQUE_FACTOR) * (1.0f - rpm / maxRpm);
    float torque = (powerKw * 1000) / (rpm * 2.0f * PI / 60.0f) * torqueCurveFactor;

    float wheelTorque = torque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;

    if (rpm >= maxRpm)
    {
        wheelTorque = 0;
    }

    float wheelForce = wheelTorque / wheelRadiusM;

    float dragForce = (AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps) / 2;
    float acceleration = (wheelForce - dragForce) / weightKg;

    speedMps += acceleration * dt;

    float wheelRpm = (speedMps / wheelRadiusM) * (60.0f / (2.0f * PI));
    rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;

    if (rpm >= maxRpm)
        rpm = maxRpm;
    else if (rpm < idleRpm)
        rpm = idleRpm;
}

void Vehicle::idle()
{
    float rollingResistance = SURFACE_ROLLING_COEFFICIENT * weightKg * GRAVITY_CONSTANT;
    float dragForce = 0.5f * AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps;
    float netDecel = (dragForce + rollingResistance) / weightKg;

    speedMps -= netDecel * dt;
    if (speedMps < 0.0f)
        speedMps = 0.0f;

    float wheelAngularDecel = netDecel / wheelRadiusM;
    float rpmDropRate = wheelAngularDecel * (60.0f / (2.0f * PI)) * gearRatios[gear - 1] * finalDriveRatio;
    rpm -= rpmDropRate * dt;
    if (rpm < idleRpm)
        rpm = idleRpm;
}

void Vehicle::brake()
{
    speedMps = 0;
}

void Vehicle::turnLeft()
{
    steeringAngleRad = maxSteeringAngleRad;
}

void Vehicle::turnRight()
{
    steeringAngleRad = -maxSteeringAngleRad;
}

void Vehicle::shiftUp()
{
    if (gear < gearCount)
    {
        rpm = rpm * gearRatios[gear] / gearRatios[gear - 1];
        gear++;
    }
}

void Vehicle::shiftDown()
{
    if (gear > 1)
    {
        rpm = rpm * gearRatios[gear - 2] / gearRatios[gear - 1];
        gear--;
    }
}

void Vehicle::updateTransmission()
{
    if (transmissionType == VE_TRANSMISSION_TYPE_AUTOMATIC)
    {
        if (rpm >= maxRpm)
        {
            shiftUp();
        }
        // Error if idleRpm is more than 90% of maxRpm
        else if (gear > 1 && maxRpm * 9 / 10 >= rpm * gearRatios[gear - 2] / gearRatios[gear - 1])
        {
            shiftDown();
        }
    }
    else
    {
        if (Input::isPressed(shiftUpKey))
        {
            shiftUp();
        }
        if (Input::isPressed(shiftDownKey))
        {
            shiftDown();
        }
    }
}

void Vehicle::calculatePhysics()
{
    if (Input::isDown(accelerateKey))
    {
        accelerate();
    }
    else if (Input::isDown(brakeKey))
    {
        brake();
    }
    else
    {
        idle();
    }

    updateTransmission();

    steeringAngleRad = 0;
    if (Input::isDown(turnLeftKey) && Input::isUp(turnRightKey))
    {
        turnLeft();
    }
    else if (Input::isDown(turnRightKey) && Input::isUp(turnLeftKey))
    {
        turnRight();
    }
}
