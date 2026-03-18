// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#include "../../shared/Log.hpp"

Vehicle::Vehicle(VehicleHandle handle, Transform transform, const VE_STRUCT_VEHICLE_CREATE_INFO &info, MeshInstanceHandle bodyMeshInstanceHandle, MeshInstanceHandle wheelFLMeshInstanceHandle, MeshInstanceHandle wheelFRMeshInstanceHandle, MeshInstanceHandle wheelBLMeshInstanceHandle, MeshInstanceHandle wheelBRMeshInstanceHandle)
    : handle(handle)
{
    this->bodyMeshInstanceHandle = bodyMeshInstanceHandle;
    this->wheelFLMeshInstanceHandle = wheelFLMeshInstanceHandle;
    this->wheelFRMeshInstanceHandle = wheelFRMeshInstanceHandle;
    this->wheelBLMeshInstanceHandle = wheelBLMeshInstanceHandle;
    this->wheelBRMeshInstanceHandle = wheelBRMeshInstanceHandle;

    wheelOffset = info.wheelOffset;

    if (info.peakTorqueNm > 0)
    {
        peakTorqueNm = info.peakTorqueNm;
    }
    else
    {
        Log::add('A', 102);
        peakTorqueNm = 300;
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

    transmissionType = info.transmissionType;

    drivetrainType = info.drivetrainType;

    if (info.brakingForceN >= 0)
    {
        brakingForceN = info.brakingForceN;
    }
    else
    {
        brakingForceN = 15000.0f;
    }

    if (info.pGearRatios)
    {
        gearRatios.resize(gearCount + 1);

        if (info.reverseGearRatio > 0.0f)
        {
            gearRatios[0] = info.reverseGearRatio;
        }
        else
        {
            gearRatios[0] = 3.5f;
        }

        std::copy(info.pGearRatios, info.pGearRatios + gearCount, gearRatios.begin() + 1);
    }
    else
    {
        if (gearCount == 1)
        {
            gearRatios[1] = 1.0f;
        }
        else
        {
            const float defaultTopRatio = 1.0f;
            const float defaultFirstRatio = 5.0f;
            gearRatios.resize(gearCount);
            for (size_t i = 1; i <= gearCount; ++i)
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

    if (info.maxSteeringAngleRad > 0 && info.maxSteeringAngleRad <= 0.9f) // Hardcoded limit
    {
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

    if (info.tireGrip > 0.05f)
    {
        tireGrip = info.tireGrip;
    }
    else
    {
        Log::add('A', 115);
        tireGrip = 1.0f;
    }

    if (info.camberRad > -(PI / 2) && info.camberRad < PI / 2)
    {
        camberRad = info.camberRad;
    }
    else
    {
        Log::add('A', 114);
        camberRad = 0;
    }

    this->transform = transform;
}

glm::mat4 Vehicle::getWheelFLMat() const
{
    glm::mat4 wheelMat =
        bodyMat *
        glm::translate(glm::mat4(1.0f), glm::vec3(wheelOffset.x, wheelOffset.y + flState.suspension, wheelOffset.z)) /*Offset & Suspension*/;

    // Steer
    wheelMat = glm::rotate(wheelMat, steeringAngleRad, glm::vec3(0, 1.0f, 0));

    // Camber
    wheelMat = glm::rotate(wheelMat, camberRad, glm::vec3(0, 0, 1));

    // Spin
    wheelMat = glm::rotate(wheelMat, flState.spin, glm::vec3(1.0f, 0, 0));

    return wheelMat;
}

glm::mat4 Vehicle::getWheelFRMat() const
{
    glm::mat4 wheelMat =
        bodyMat *
        glm::translate(glm::mat4(1.0f), glm::vec3(-wheelOffset.x, wheelOffset.y + frState.suspension, wheelOffset.z)) /*Offset & Suspension*/ *
        glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0, 1, 0)) /*Invert*/;

    // Steer
    wheelMat = glm::rotate(wheelMat, steeringAngleRad, glm::vec3(0, 1.0f, 0));

    // Camber
    wheelMat = glm::rotate(wheelMat, camberRad, glm::vec3(0, 0, 1));

    // Spin
    wheelMat = glm::rotate(wheelMat, frState.spin, glm::vec3(-1.0f, 0, 0));

    return wheelMat;
}

glm::mat4 Vehicle::getWheelBLMat() const
{
    glm::mat4 wheelMat =
        bodyMat *
        glm::translate(glm::mat4(1.0f), glm::vec3(wheelOffset.x, wheelOffset.y + blState.suspension, -wheelOffset.z)) /*Offset & Suspension*/;

    // Camber
    wheelMat = glm::rotate(wheelMat, camberRad, glm::vec3(0, 0, 1));

    // Spin
    wheelMat = glm::rotate(wheelMat, blState.spin, glm::vec3(1.0f, 0, 0));

    return wheelMat;
}

glm::mat4 Vehicle::getWheelBRMat() const
{
    glm::mat4 wheelMat =
        bodyMat *
        glm::translate(glm::mat4(1.0f), glm::vec3(-wheelOffset.x, wheelOffset.y + brState.suspension, -wheelOffset.z)) /*Offset & Suspension*/ *
        glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0, 1, 0)) /*Invert*/;

    // Camber
    wheelMat = glm::rotate(wheelMat, camberRad, glm::vec3(0, 0, 1));

    // Spin
    wheelMat = glm::rotate(wheelMat, brState.spin, glm::vec3(-1.0f, 0, 0));

    return wheelMat;
}

void Vehicle::collideVelocityVector(glm::vec3 localPOI)
{
    glm::vec3 normalizedPOI = -localPOI;

    // Ignore Y
    normalizedPOI.y = 0.0f;

    normalizedPOI = glm::normalize(normalizedPOI);

    glm::mat4 R =
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.yaw, glm::vec3(0, 1, 0)) *
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.pitch, glm::vec3(1, 0, 0)) *
        glm::rotate(glm::mat4(1.0f), (float)transform.rotation.roll, glm::vec3(0, 0, 1));

    glm::vec3 collisionNormal = glm::normalize(glm::vec3(R * glm::vec4(normalizedPOI, 0.0f)));

    float velocityAlongNormal = glm::dot(velocityMps, collisionNormal);
    if (velocityAlongNormal > 0.0f)
    {
        collisionNormal = -collisionNormal;
        velocityAlongNormal = -velocityAlongNormal;
    }

    if (velocityAlongNormal < 0.0f)
        velocityMps -= collisionNormal * velocityAlongNormal;
}

void Vehicle::printState()
{
    std::cout << (std::round(forwardSpeedMps * 3.6f) > 1.0f ? std::round(forwardSpeedMps * 3.6f) : 0.0f) << " km/h | " << std::round(rpm) << " rpm | " << (isNeutral ? "N" : (gear == 0 ? "R" : std::to_string(gear))) << " gear" << std::endl;
}

void Vehicle::printVIS()
{
    printf("%.2ft %.2fb %.2fc %.2fs\n", vis.throttle, vis.brake, vis.clutch, vis.steer);
}

void Vehicle::updateTransform()
{
    transform.position.x += velocityMps.x * dt;
    transform.position.y += velocityMps.y * dt;
    transform.position.z += velocityMps.z * dt;

    transform.rotation.yaw += yawRateRadps * (float)dt;

    transformMat = glm::mat4(1.0f);
    transformMat = glm::translate(transformMat, glm::vec3{transform.position.x, transform.position.y, transform.position.z});
    transformMat = glm::rotate(transformMat, (float)transform.rotation.roll, glm::vec3(0, 0, 1));
    transformMat = glm::rotate(transformMat, (float)transform.rotation.yaw, glm::vec3(0, 1, 0));
    transformMat = glm::rotate(transformMat, (float)transform.rotation.pitch, glm::vec3(1, 0, 0));

    bodyMat = transformToMat(transform);
}

void Vehicle::tick(VehicleInputState vis, Environment environment, float surfaceFriction, ve_time_t deltaTime)
{
    dt = deltaTime;

    this->vis = vis;
    rawThrottle = vis.throttle;

    if (vis.starter)
        activateStarter();

    flState.grip = surfaceFriction;
    frState.grip = surfaceFriction;
    blState.grip = surfaceFriction;
    brState.grip = surfaceFriction;

    steer();

    updateTransmission();

    stallAssist();

    cruiseControl();

    calcForces(environment);

    calcTireTemperatures(environment);

    // printState();
    // printVIS();
}