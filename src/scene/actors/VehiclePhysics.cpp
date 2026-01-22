// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define TORQUE_CONVERSION_CONSTANT 5252

#define BASELINE_TORQUE_FACTOR 0.9f
#define SURFACE_ROLLING_COEFFICIENT 0.015f

void Vehicle::stallAssist()
{
    if (rpm >= maxRpm)
    {
        vis.throttle = 0.0f;
    }
    else if (rpm < idleRpm && vis.throttle < 0.0001f / dt)
    {
        vis.throttle = 0.0001f / dt;
    }
}

float Vehicle::calcFDriveMag()
{
    float engineAngularSpeed = (2 * PI * rpm) / 60;

    float powerW = powerKw * 1000;

    float powerAtCurrRPM = powerW * vis.throttle;

    float torqueCurveFactor = BASELINE_TORQUE_FACTOR + (1.0f - BASELINE_TORQUE_FACTOR) * (1.0f - rpm / maxRpm);

    float engineTorque = (powerAtCurrRPM * torqueCurveFactor) / std::max(engineAngularSpeed, 1.0f);

    float wheelTorque = engineTorque * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;

    return wheelTorque / wheelRadiusM;
}

void Vehicle::calcForces(const Environment &environment, float surfaceFriction)
{
    glm::mat4 R =
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.yaw, glm::vec3(0, 1, 0)) *
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.pitch, glm::vec3(1, 0, 0)) *
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.roll, glm::vec3(0, 0, 1));

    glm::vec3 forward = glm::normalize(glm::vec3(R * glm::vec4(0, 0, 1, 0)));
    glm::vec3 right = glm::normalize(glm::vec3(R * glm::vec4(1, 0, 0, 0)));
    glm::vec3 up = glm::normalize(glm::vec3(R * glm::vec4(0, 1, 0, 0)));

    float FDriveMag = calcFDriveMag();

    glm::vec3 FDrive = forward * FDriveMag;

    float FDragMag = 0.5f * environment.airDensity * dragCoeff * frontalAreaM2 * speedMps * speedMps;

    glm::vec3 FDrag(0.0f);
    if (glm::length(velocityMps) > 0.01f)
    {
        FDrag = -(velocityMps / glm::length(velocityMps)) * FDragMag;
    }

    float FRollMag = SURFACE_ROLLING_COEFFICIENT * weightKg * environment.gravity * (speedMps == 0 ? 0 : 1);

    glm::vec3 FRoll(0.0f);
    if (glm::length(velocityMps) > 0.01f)
    {
        FRoll = -(velocityMps / glm::length(velocityMps)) * FRollMag;
    }

    float FBrakeMag = vis.brake * brakingForce;

    glm::vec3 FBrake(0.0f);
    {
        if (glm::length(velocityMps) > 0.01f)
            FBrake = -(velocityMps / glm::length(velocityMps)) * FBrakeMag;
    }

    glm::vec3 FLat(0.0f);
    {
        float forwardSpeedMps = glm::dot(velocityMps, forward);
        float lateralSpeedMps = glm::dot(velocityMps, right);

        float centerOfGravity = wheelOffset.z / 2;

        float frontSlipAngleRad = std::atan2(lateralSpeedMps + centerOfGravity * yawRateRadps, forwardSpeedMps) - steeringAngleRad;

        float rearSlipAngleRad = std::atan2(lateralSpeedMps - centerOfGravity * yawRateRadps, forwardSpeedMps);

        float frictionCoefficient = tireGrip * surfaceFriction * (1.0f + std::fabs(camberRad));

        float totalNormalForceN = weightKg * environment.gravity;
        float frontAxleNormalForceN = 0.5f * totalNormalForceN;
        float rearAxleNormalForceN = 0.5f * totalNormalForceN;

        const float frontCorneringStiffnessNPerRad = 80000.0f;
        const float rearCorneringStiffnessNPerRad = 90000.0f;

        float frontLateralForceN = -frontCorneringStiffnessNPerRad * frontSlipAngleRad;
        float rearLateralForceN = -rearCorneringStiffnessNPerRad * rearSlipAngleRad;

        float maxFrontLateralForceN = frictionCoefficient * frontAxleNormalForceN;
        float maxRearLateralForceN = frictionCoefficient * rearAxleNormalForceN;

        frontLateralForceN = maxFrontLateralForceN * std::tanh(frontLateralForceN / maxFrontLateralForceN);

        rearLateralForceN = maxRearLateralForceN * std::tanh(rearLateralForceN / maxRearLateralForceN);

        FLat = right * (frontLateralForceN + rearLateralForceN);

        float yawMomentNm = centerOfGravity * (frontLateralForceN - rearLateralForceN);

        const float yawInertiaKgM2 = 2500.0f;

        float yawAccelerationRadps2 = yawMomentNm / yawInertiaKgM2;

        yawRateRadps += yawAccelerationRadps2 * (float)dt;
    }

    const float slope = std::atan(std::sqrt(std::tan(transform.rotation.pitch) * std::tan(transform.rotation.pitch) + std::tan(transform.rotation.roll) * std::tan(transform.rotation.roll)));
    float FSlopeMag = weightKg * environment.gravity * sin(slope);

    glm::vec3 FSlope(0.0f);
    {
        FSlope = -forward * FSlopeMag;
    }

    glm::vec3 FGravity(0.0f, -weightKg * environment.gravity, 0.0f);

    glm::vec3 FTotal = FDrive + FDrag + FRoll + FBrake + FLat + FSlope; // + FGravity;

    glm::vec3 accel = FTotal / weightKg;
    velocityMps += accel * (float)dt;

    speedMps = glm::length(velocityMps);
}

void Vehicle::calcRpm()
{
    float wheelRpm = (speedMps / wheelRadiusM) * (60.0f / (2.0f * PI));
    rpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;
}

void Vehicle::steer()
{
    const float k = 0.0025f;
    float speedFactor = 1.0f / (1.0f + k * speedMps * speedMps);
    speedFactor = clamp(speedFactor, 0.15f, 1.0f);

    float target = vis.steer * maxSteeringAngleRad * speedFactor;

    steeringAngleRad += target - steeringAngleRad;
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
        if (vis.shiftUp)
        {
            shiftUp();
        }
        if (vis.shiftDown)
        {
            shiftDown();
        }
    }
}

void Vehicle::updateTransform()
{
    transform.position.x += velocityMps.x * dt;
    transform.position.y += velocityMps.y * dt;
    transform.position.z += velocityMps.z * dt;

    transform.rotation.yaw += yawRateRadps * (float)dt;
}

void Vehicle::calculatePhysics(Environment environment, float surfaceFriction)
{
    steer();

    stallAssist();

    calcForces(environment, surfaceFriction);

    calcRpm();

    updateTransmission();

    updateTransform();
}
