// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define TORQUE_CONVERSION_CONSTANT 5252
#define GRAVITY_CONSTANT 9.81f

#define AIR_DENSITY 1.225f

#define BASELINE_TORQUE_FACTOR 0.9f
#define SURFACE_ROLLING_COEFFICIENT 0.015f

void Vehicle::calcForces()
{
    if (rpm >= maxRpm)
    {
        throttleState = 0.0f;
    }
    else if (rpm < idleRpm && throttleState < 0.0001f / dt)
    {
        throttleState = 0.0001f / dt;
    }

    glm::vec3 forward;
    forward.x = sin(transform.rotation.yaw);
    forward.y = 0.0f;
    forward.z = cos(transform.rotation.yaw);
    forward = glm::normalize(forward);

    glm::vec3 right;
    right.x = cos(transform.rotation.yaw);
    right.y = 0.0f;
    right.z = -sin(transform.rotation.yaw);
    right = glm::normalize(right);

    speedMps = glm::length(glm::vec3(velocityMps.x, 0.0f, velocityMps.z));

    float engineAngularSpeed = (2 * PI * rpm) / 60;

    float powerW = powerKw * 1000;

    float powerAtCurrRPM = powerW * throttleState;

    float torqueCurveFactor = BASELINE_TORQUE_FACTOR + (1.0f - BASELINE_TORQUE_FACTOR) * (1.0f - rpm / maxRpm);

    float engineTorque = (powerAtCurrRPM * torqueCurveFactor) / std::max(engineAngularSpeed, 1.0f);

    float wheelTorque = engineTorque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;

    float FDriveMag = wheelTorque / wheelRadiusM;

    glm::vec3 FDrive = forward * FDriveMag;

    float FDragMag = 0.5f * AIR_DENSITY * dragCoeff * frontalAreaM2 * speedMps * speedMps;

    glm::vec3 FDrag(0.0f);
    if (speedMps > 0.01f)
    {
        FDrag = -(velocityMps / speedMps) * FDragMag;
    }

    float FRollMag = SURFACE_ROLLING_COEFFICIENT * weightKg * GRAVITY_CONSTANT * (speedMps == 0 ? 0 : 1);

    glm::vec3 FRoll(0.0f);
    if (speedMps > 0.01f)
    {
        FRoll = -(velocityMps / speedMps) * FRollMag;
    }

    float FBrakeMag = brakeState * brakingForce;

    glm::vec3 FBrake(0.0f);
    {
        FBrake = -forward * (glm::sign(glm::dot(velocityMps, forward)) * FBrakeMag);
    }

    const float slope = std::atan(std::sqrt(std::tan(glm::radians(transform.rotation.pitch)) * std::tan(glm::radians(transform.rotation.pitch)) + std::tan(glm::radians(transform.rotation.roll)) * std::tan(glm::radians(transform.rotation.roll))));
    float FSlopeMag = weightKg * GRAVITY_CONSTANT * sin(slope);

    glm::vec3 FSlope(0.0f);
    {
        FSlope = -forward * FSlopeMag;
    }

    glm::vec3 FLat(0.0f);
    {
        float currentTireGrip = tireGrip * (1.0f + fabs(camberRad));

        float maxLat = currentTireGrip * weightKg * GRAVITY_CONSTANT;

        float FLatMag = -(currentTireGrip * 1000) * glm::dot(velocityMps, right); // (currentTireGrip * 1000) is corner stiffness
        FLatMag = glm::clamp(FLatMag, -maxLat, maxLat);

        FLat = right * FLatMag;
    }

    glm::vec3 FTotal = FDrive + FDrag + FRoll + FBrake + FSlope + FLat;

    glm::vec3 accel = FTotal / weightKg;
    velocityMps += accel * (float)dt;
    velocityMps.y = 0.0f;

    speedMps = glm::length(glm::vec3(velocityMps.x, 0.0f, velocityMps.z));

    if (speedMps < 0)
        speedMps = 0;
}

void Vehicle::calcRpm()
{
    float wheelRpm = (speedMps / wheelRadiusM) * (60.0f / (2.0f * PI));
    rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;
}

void Vehicle::steer(float turningInput)
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
        steer(1.0f);
    }
    else if (Input::isDown(turnRightKey) && Input::isUp(turnLeftKey))
    {
        steer(-1.0f);
    }
}

void Vehicle::updateTransform()
{
    transform.position.x += velocityMps.x * dt;
    transform.position.y += velocityMps.y * dt;
    transform.position.z += velocityMps.z * dt;
}

void Vehicle::calculatePhysics()
{
    handleInput();

    calcForces();

    calcRpm();

    updateTransmission();

    updateTransform();
}
