// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define TORQUE_CONVERSION_CONSTANT 5252
#define GRAVITY_CONSTANT 9.81f

#define AIR_DENSITY 1.225f

#define BASELINE_TORQUE_FACTOR 0.9f
#define SURFACE_ROLLING_COEFFICIENT 0.015f

void Vehicle::calcSpeed()
{
    float engineAngularSpeed = (2 * PI * rpm) / 60;

    float wheelAngularSpeed = engineAngularSpeed / (gearRatios[gear - 1] * finalDriveRatio);

    float powerW = powerKw * 1000;
    // Temporary constant
    float powerAtCurrRPM = powerW * throttleState;

    // float torqueCurveFactor = BASELINE_TORQUE_FACTOR + (1.0f - BASELINE_TORQUE_FACTOR) * (1.0f - rpm / maxRpm);
    float engineTorqueFromPower = powerAtCurrRPM /* * torqueCurveFactor*/ / std::max(engineAngularSpeed, 0.01f);

    float wheelTorque = engineTorqueFromPower * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;

    float FDrive = wheelTorque / wheelRadiusM;

    float FDrag = 0.5f * AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps;

    float FRoll = SURFACE_ROLLING_COEFFICIENT * weightKg * GRAVITY_CONSTANT * (speedMps == 0 ? 0 : 1);

    const float slope = std::atan(std::sqrt(std::tan(glm::radians(rotation.pitch)) * std::tan(glm::radians(rotation.pitch)) + std::tan(glm::radians(rotation.roll)) * std::tan(glm::radians(rotation.roll))));

    float FSlope = weightKg * GRAVITY_CONSTANT * sin(slope);

    float FBrake = brakeState * brakingForce;

    float a = (FDrive - FDrag - FRoll - FSlope - FBrake) / weightKg;

    speedMps += a * dt;

    if (speedMps < 0)
        speedMps = 0;
}

void Vehicle::calcRpm()
{
    float wheelRpm = (speedMps / wheelRadiusM) * (60.0f / (2.0f * PI));
    rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;

    if (rpm >= maxRpm)
        rpm = maxRpm;
    else if (rpm < idleRpm)
        rpm = idleRpm;
}

void Vehicle::turnLeft()
{
    float decrease = 0;
    decrease = speedMps * 3.6f * maxSteeringAngleRad / 200;

    if (decrease > maxSteeringAngleRad / 3)
        decrease = maxSteeringAngleRad / 2;

    const float wheelBase = 2.6f;
    steeringAngleRad = maxSteeringAngleRad - decrease;
    double turnRate = speedMps * tan(steeringAngleRad) / wheelBase;
    rotation.yaw += turnRate * dt / 5.0f;
}

void Vehicle::turnRight()
{
    float decrease = 0;
    decrease = speedMps * 3.6f * maxSteeringAngleRad / 200;

    if (decrease > maxSteeringAngleRad / 3)
        decrease = maxSteeringAngleRad / 2;

    const float wheelBase = 2.6f;
    steeringAngleRad = -maxSteeringAngleRad + decrease;
    double turnRate = speedMps * tan(steeringAngleRad) / wheelBase;
    rotation.yaw += turnRate * dt / 5.0f;
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

void Vehicle::handleInput()
{
    if (Input::isDown(accelerateKey))
        throttleState = 1.0f;
    else
        throttleState = 0.0f;

    if (Input::isDown(brakeKey))
        brakeState = 1.0f;
    else
        brakeState = 0.0f;

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

void Vehicle::calculatePhysics()
{
    handleInput();

    calcSpeed();

    calcRpm();

    updateTransmission();
}
