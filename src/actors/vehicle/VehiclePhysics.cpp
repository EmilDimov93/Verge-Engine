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
    if (rpm >= maxRpm){
        throttleState = 0.0f;
    }
    else if (rpm < idleRpm){
        throttleState = 0.0001f / dt;
    }

    float engineAngularSpeed = (2 * PI * rpm) / 60;

    float powerW = powerKw * 1000;

    float powerAtCurrRPM = powerW * throttleState;

    float torqueCurveFactor = BASELINE_TORQUE_FACTOR + (1.0f - BASELINE_TORQUE_FACTOR) * (1.0f - rpm / maxRpm);

    float engineTorque = (powerAtCurrRPM * torqueCurveFactor) / std::max(engineAngularSpeed, 1.0f);

    float wheelTorque = engineTorque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;

    float FDrive = wheelTorque / wheelRadiusM;

    float FDrag = 0.5f * AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps;

    float FRoll = SURFACE_ROLLING_COEFFICIENT * weightKg * GRAVITY_CONSTANT * (speedMps == 0 ? 0 : 1);

    const float slope = std::atan(std::sqrt(std::tan(glm::radians(transform.rotation.pitch)) * std::tan(glm::radians(transform.rotation.pitch)) + std::tan(glm::radians(transform.rotation.roll)) * std::tan(glm::radians(transform.rotation.roll))));

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
}

void Vehicle::turn(float turningInput)
{
    const float k = 0.005f;
    float speedFactor = 1.0f / (1.0f + k * speedMps * speedMps);

    turningInput = clamp(turningInput, -1.0f, 1.0f);
    speedFactor = clamp(speedFactor, 0.0f, 1.0f);
    steeringAngleRad = turningInput * maxSteeringAngleRad * speedFactor;

    const float wheelBase = 2.6f;
    transform.rotation.yaw += speedMps * tan(steeringAngleRad) / wheelBase * dt;
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
        turn(1.0f);
    }
    else if (Input::isDown(turnRightKey) && Input::isUp(turnLeftKey))
    {
        turn(-1.0f);
    }
}

void Vehicle::updateTransform()
{
    // Temporary
    moveDirection = transform.rotation;

    double cosPitch = cos(moveDirection.pitch);
    double sinPitch = sin(moveDirection.pitch);
    double cosYaw = cos(moveDirection.yaw);
    double sinYaw = sin(moveDirection.yaw);

    double fx = cosPitch * sinYaw;
    double fy = -sinPitch;
    double fz = cosPitch * cosYaw;

    transform.position.x += fx * speedMps * dt;
    transform.position.y += fy * speedMps * dt;
    transform.position.z += fz * speedMps * dt;
}

void Vehicle::calculatePhysics()
{
    handleInput();

    calcSpeed();

    calcRpm();

    updateTransmission();

    updateTransform();
}
