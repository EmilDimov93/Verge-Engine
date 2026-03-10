// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define RADPS_TO_RPM_CONVERSION_FACTOR 60.0f / (2.0f * PI)

#define SURFACE_ROLLING_COEFFICIENT 0.015f

#define ENGINE_INERTIA 0.1f
#define ENGINE_FRICTION_COEFF 0.0012f

void Vehicle::activateStarter()
{
    const float maxStarterSpeed = 300.0f;

    if (rpm < maxStarterSpeed)
        rpm = maxStarterSpeed;
}

void Vehicle::stallAssist()
{
    if (rpm >= maxRpm)
    {
        vis.throttle = 0.0f;
    }
    else if (rpm < idleRpm)
    {
        if (rpm < idleRpm)
            isNeutral = true;

        float minThrottle = clamp01(0.0001f * (idleRpm - rpm) / dt);

        if (vis.throttle < minThrottle)
            vis.throttle = minThrottle;
    }

    const float maxBackwardAssistSpeedMps = 5.0f / 3.6f;
    const float maxForwardAssistSpeedMps = 3.0f / 3.6f;
    if (transmissionType != VE_TRANSMISSION_TYPE_MANUAL_WITH_CLUTCH && 
        forwardSpeedMps > -maxBackwardAssistSpeedMps && 
        forwardSpeedMps < maxForwardAssistSpeedMps && 
        vis.clutch == 0.0f)
    {
        vis.clutch = 1.0f - (0.1f + (forwardSpeedMps + maxBackwardAssistSpeedMps) * 3.6f / 10.0f);
    }
}

void Vehicle::cruiseControl()
{
    if (vis.brake != 0)
        cruiseControlTargetMps = 0;

    if (cruiseControlTargetMps == 0)
        return;

    if (forwardSpeedMps < cruiseControlTargetMps)
    {
        float minThrottle = clamp01(0.01f * (cruiseControlTargetMps - forwardSpeedMps) / dt);

        vis.throttle = (minThrottle < vis.throttle ? vis.throttle : minThrottle);
    }

    else if (forwardSpeedMps > cruiseControlTargetMps && vis.throttle == 0)
        vis.brake = 1.0f;
}

float Vehicle::getTorque()
{
    float rpmNorm = rpm / (maxRpm + maxRpm / 4);
    float torqueCurve = rpmNorm * (1.0f - rpmNorm) * 4.0f;

    return peakTorqueNm * torqueCurve;
}

float Vehicle::calcFDriveMag()
{
    const float drivetrainEngagement = isNeutral ? 0.0f : (1.0f - vis.clutch);

    const float gearRatio = gearRatios[gear - 1];

    {
        wheelRpm = drivetrainEngagement * rpm / (gearRatio * finalDriveRatio) + (1.0f - drivetrainEngagement) * (fabs(forwardSpeedMps) * RADPS_TO_RPM_CONVERSION_FACTOR / wheelRadiusM);
        wheelSpin = std::fmod(wheelSpin + wheelRpm * dt * RPM_TO_RADPS_CONVERSION_FACTOR, 2.0f * PI);
    }

    const float clutchSlipRadps = rpm * RPM_TO_RADPS_CONVERSION_FACTOR - forwardSpeedMps * gearRatio * finalDriveRatio / wheelRadiusM;

    const float engineTorqueNm = getTorque() * vis.throttle;
    const float frictionTorqueNm = ENGINE_FRICTION_COEFF * rpm * RPM_TO_RADPS_CONVERSION_FACTOR;

    const float clutchStiffnessNmPerRadps = 12.0f;
    const float clutchCapacityNmAtFullEngagement = 500.0f;
    const float clutchTorqueCapacityNm = drivetrainEngagement * clutchCapacityNmAtFullEngagement;

    const float clutchTorqueNm = clamp(clutchStiffnessNmPerRadps * clutchSlipRadps, -clutchTorqueCapacityNm, clutchTorqueCapacityNm);

    const float angularAccelRadps = (engineTorqueNm - frictionTorqueNm - clutchTorqueNm) / ENGINE_INERTIA;

    rpm += angularAccelRadps * dt * RADPS_TO_RPM_CONVERSION_FACTOR;

    if (rpm < 0)
        rpm = 0;

    const float wheelTorqueNm = clutchTorqueNm * gearRatio * finalDriveRatio * drivetrainEfficiency;

    return wheelTorqueNm / wheelRadiusM;
}

void Vehicle::calcForces(const Environment &environment)
{
    const glm::mat4 R =
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.yaw, glm::vec3(0, 1, 0)) *
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.pitch, glm::vec3(1, 0, 0)) *
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.roll, glm::vec3(0, 0, 1));

    glm::vec3 forward = glm::normalize(glm::vec3(R * glm::vec4(0, 0, 1, 0)));
    glm::vec3 right = glm::normalize(glm::vec3(R * glm::vec4(1, 0, 0, 0)));
    glm::vec3 up = glm::normalize(glm::vec3(R * glm::vec4(0, 1, 0, 0)));

    float FDriveMag = calcFDriveMag();

    glm::vec3 FDrive = forward * FDriveMag;

    float FDragMag = 0.5f * environment.airDensityKgpm3 * dragCoeff * frontalAreaM2 * speedMps * speedMps;

    glm::vec3 FDrag(0.0f);
    if (glm::length(velocityMps) > 0.01f)
    {
        FDrag = -(velocityMps / glm::length(velocityMps)) * FDragMag;
    }

    float FRollMag = SURFACE_ROLLING_COEFFICIENT * weightKg * environment.gravityMps2 * (forwardSpeedMps < 0.01f ? 0 : 1);

    glm::vec3 FRoll(0.0f);
    if (glm::length(velocityMps) > 0.01f)
    {
        FRoll = -(velocityMps / glm::length(velocityMps)) * FRollMag;
    }

    float FBrakeMag = clamp01(vis.brake + vis.handbrake) * brakingForce;

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

        float backSlipAngleRad = std::atan2(lateralSpeedMps - centerOfGravity * yawRateRadps, forwardSpeedMps);

        float frontFrictionCoefficient = tireGrip * (flState.grip + frState.grip) / 2 * (1.0f + std::fabs(camberRad));
        float backFrictionCoefficient = tireGrip * (blState.grip + brState.grip) / 2 * (1.0f + std::fabs(camberRad));

        const float handbrakeRearGripScale = 0.75f;
        backFrictionCoefficient *= (1.0f - vis.handbrake * handbrakeRearGripScale);

        float totalNormalForceN = weightKg * environment.gravityMps2;
        float frontAxleNormalForceN = 0.5f * totalNormalForceN;
        float backAxleNormalForceN = 0.5f * totalNormalForceN;

        float maxFrontLateralForceN = frontFrictionCoefficient * frontAxleNormalForceN;
        float maxBackLateralForceN = backFrictionCoefficient * backAxleNormalForceN;

        const float frontCorneringStiffnessNPerRad = 80000.0f;
        const float backCorneringStiffnessNPerRad = 80000.0f;

        float frontLateralForceN = -frontCorneringStiffnessNPerRad * frontSlipAngleRad;
        float backLateralForceN = -backCorneringStiffnessNPerRad * backSlipAngleRad;

        frontLateralForceN = maxFrontLateralForceN * std::tanh(frontLateralForceN / AvoidZero(maxFrontLateralForceN));
        backLateralForceN = maxBackLateralForceN * std::tanh(backLateralForceN / AvoidZero(maxBackLateralForceN));

        glm::vec3 frontRight = glm::normalize(right * std::cos(steeringAngleRad) - forward * std::sin(steeringAngleRad));
        glm::vec3 frontLateralForce = frontRight * frontLateralForceN;
        glm::vec3 backLateralForce = right * backLateralForceN;
        FLat = frontLateralForce + backLateralForce;

        float yawMomentNm = centerOfGravity * (frontLateralForceN - backLateralForceN);

        const float yawInertiaKgM2 = 2500.0f;

        float yawAccelerationRadps2 = yawMomentNm / yawInertiaKgM2;

        yawRateRadps += yawAccelerationRadps2 * (float)dt;
    }

    const float slope = std::atan(std::sqrt(std::tan(transform.rotation.pitch) * std::tan(transform.rotation.pitch) + std::tan(transform.rotation.roll) * std::tan(transform.rotation.roll)));
    float FSlopeMag = weightKg * environment.gravityMps2 * sin(slope);

    glm::vec3 FSlope(0.0f);
    {
        FSlope = -forward * FSlopeMag;
    }

    glm::vec3 FGravity(0.0f, -weightKg * environment.gravityMps2, 0.0f);

    glm::vec3 FTotal = FDrive + FDrag + FRoll + FBrake + FLat + FSlope + FGravity;

    glm::vec3 accel = FTotal / weightKg;
    velocityMps += accel * (float)dt;

    speedMps = glm::length(velocityMps);
    forwardSpeedMps = glm::dot(velocityMps, forward);
}

void Vehicle::steer()
{
    const float linearAttenuationCoefficient = 0.1f;
    const float quadraticAttenuationCoefficient = 0.00025f;
    const float steerSpeed = 10.0f;
    float speedFactor = 1.0f / (1.0f + linearAttenuationCoefficient * forwardSpeedMps + quadraticAttenuationCoefficient * forwardSpeedMps * forwardSpeedMps);
    speedFactor = clamp(speedFactor, 0.15f, 1.0f);

    float target = vis.steer * maxSteeringAngleRad * speedFactor;

    steeringAngleRad = steeringAngleRad + (target - steeringAngleRad) * steerSpeed * dt;

    clamp(steeringAngleRad, -maxSteeringAngleRad, maxSteeringAngleRad);
}

void Vehicle::shiftUp()
{
    if (gear < gearCount)
    {
        rpm = rpm * gearRatios[gear] / gearRatios[gear - 1];
        gear++;
    }
    if (isNeutral)
    {
        gear = 1;
        isNeutral = false;
    }
}

void Vehicle::shiftDown()
{
    if (gear > 1)
    {
        rpm = rpm * gearRatios[gear - 2] / gearRatios[gear - 1];
        gear--;
    }
    else if (gear == 1)
    {
        isNeutral = true;
    }
}

void Vehicle::updateTransmission()
{
    if (transmissionType == VE_TRANSMISSION_TYPE_AUTOMATIC)
    {
        if (vis.clutch == 1.0f)
            return;

        if (isNeutral)
        {
            if (vis.throttle == 0.0f)
                return;
            else
                isNeutral = false;
        }

        // Temporary(unstable)
        if (rpm >= maxRpm - 500 && rpm * RPM_TO_RADPS_CONVERSION_FACTOR <= fabs(forwardSpeedMps) * gearRatios[gear - 1] * finalDriveRatio / wheelRadiusM)
        {
            shiftUp();
        }
        else if (gear > 1 && idleRpm + (maxRpm - idleRpm) * 7 / 10 >= rpm * gearRatios[gear - 2] / gearRatios[gear - 1])
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
