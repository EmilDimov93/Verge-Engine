// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#define TORQUE_CONVERSION_CONSTANT 5252
#define RADPS_TO_RPM_CONVERSION_FACTOR 60.0f / (2.0f * PI)

#define BASELINE_TORQUE_FACTOR 0.9f
#define SURFACE_ROLLING_COEFFICIENT 0.015f

#define ENGINE_INERTIA 0.1f
#define ENGINE_FRICTION_COEFF 0.0012f

void Vehicle::start()
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
        vis.clutch = 1.0f;
        const float minThrottle = 0.0001f * (idleRpm - rpm) / dt;
        if(vis.throttle < minThrottle)
            vis.throttle = minThrottle;
    }
}

void Vehicle::cruiseControl()
{
    if (vis.brake != 0)
        cruiseControlTargetMps = 0;

    if (cruiseControlTargetMps == 0)
        return;

    if (forwardSpeedMps < cruiseControlTargetMps){
        float minThrottle = 0.01f * (cruiseControlTargetMps - forwardSpeedMps) / dt;
        vis.throttle = (minThrottle < vis.throttle ? vis.throttle : minThrottle);
    }

    else if (forwardSpeedMps > cruiseControlTargetMps && vis.throttle == 0)
        vis.brake = 1.0f;

    if(vis.throttle > 1.0f)
        vis.throttle = 1.0f;
}

float Vehicle::getTorque()
{
    float rpmNorm = rpm / (maxRpm + maxRpm / 4);
    float torqueCurve = rpmNorm * (1.0f - rpmNorm) * 4.0f;

    return peakTorqueNm * torqueCurve;
}

float Vehicle::calcFDriveMag()
{
    float engineTorqueNm = getTorque() * vis.throttle;

    float frictionTorqueNm = ENGINE_FRICTION_COEFF * rpm;
    float angularAccel = (engineTorqueNm - frictionTorqueNm) / ENGINE_INERTIA;
    rpm += angularAccel * dt * RADPS_TO_RPM_CONVERSION_FACTOR;

    float wheelRpm = (fabs(forwardSpeedMps) / wheelRadiusM) * RADPS_TO_RPM_CONVERSION_FACTOR;

    float connectedClutchRpm = wheelRpm * gearRatios[gear - 1] * finalDriveRatio;

    rpm = vis.clutch * rpm + (1.0f - vis.clutch) * (connectedClutchRpm + rpm) / 2;

    float wheelTorque = (1.0f - vis.clutch) * engineTorqueNm * gearRatios[gear - 1] * finalDriveRatio * drivetrainEfficiency;

    return wheelTorque / wheelRadiusM;
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

    float totalBrake = vis.brake + (vis.handbrake ? 1.0f : 0.0f);
    if (totalBrake > 1.0f)
        totalBrake = 1.0f;
    float FBrakeMag = totalBrake * brakingForce;

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

        FLat = right * (frontLateralForceN + backLateralForceN);

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
    const float quadraticAttenuationCoefficient  = 0.00025f;
    const float steerSpeed = 10.0f;
    float speedFactor = 1.0f / (1.0f + linearAttenuationCoefficient * forwardSpeedMps + quadraticAttenuationCoefficient * forwardSpeedMps * forwardSpeedMps);
    speedFactor = clamp(speedFactor, 0.15f, 1.0f);

    float target = vis.steer * maxSteeringAngleRad * speedFactor;

    steeringAngleRad = steeringAngleRad + (target - steeringAngleRad) * steerSpeed * dt;
    
    if(steeringAngleRad > maxSteeringAngleRad)
        steeringAngleRad = maxSteeringAngleRad;
    if(steeringAngleRad < -maxSteeringAngleRad)
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
        if (vis.clutch == 1.0f)
            return;
        
        // Temporary(unstable)
        if (rpm >= maxRpm - 500)
        {
            shiftUp();
        }
        else if (gear > 1 && idleRpm + (maxRpm - idleRpm) * 9 / 10 >= rpm * gearRatios[gear - 2] / gearRatios[gear - 1])
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
