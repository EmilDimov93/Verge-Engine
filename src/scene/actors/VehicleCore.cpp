// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

#include "../../shared/Log.hpp"

#define HP_TO_KW_CONVERSION_FACTOR 0.7457f

Vehicle::Vehicle(VehicleHandle handle, Transform transform, const VE_STRUCT_VEHICLE_CREATE_INFO &info, MeshInstanceHandle bodyMeshInstanceHandle, MeshInstanceHandle wheelFLMeshInstanceHandle, MeshInstanceHandle wheelFRMeshInstanceHandle, MeshInstanceHandle wheelBLMeshInstanceHandle, MeshInstanceHandle wheelBRMeshInstanceHandle)
    : handle(handle)
{
    this->bodyMeshInstanceHandle = bodyMeshInstanceHandle;
    this->wheelFLMeshInstanceHandle = wheelFLMeshInstanceHandle;
    this->wheelFRMeshInstanceHandle = wheelFRMeshInstanceHandle;
    this->wheelBLMeshInstanceHandle = wheelBLMeshInstanceHandle;
    this->wheelBRMeshInstanceHandle = wheelBRMeshInstanceHandle;

    wheelOffset = info.wheelOffset;

    if (info.power > 0)
    {
        switch (info.powerUnit)
        {
        case VE_POWER_UNIT_KILOWATTS:
            powerKw = info.power;
            break;
        case VE_POWER_UNIT_HORSEPOWER:
            powerKw = HP_TO_KW_CONVERSION_FACTOR * info.power;
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

    transmissionType = info.transmissionType;

    if (info.brakingForce >= 0)
    {
        brakingForce = info.brakingForce;
    }
    else
    {
        brakingForce = 1.0f;
    }

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
        glm::translate(glm::mat4(1.0f), glm::vec3(wheelOffset.x, wheelOffset.y, wheelOffset.z)) /*Offset*/;

    // Steer
    wheelMat = glm::rotate(wheelMat, steeringAngleRad, glm::vec3(0, 1.0f, 0));

    // Camber
    wheelMat = glm::rotate(wheelMat, camberRad, glm::vec3(0, 0, 1));

    // Spin
    wheelMat = glm::rotate(wheelMat, wheelSpin, glm::vec3(1.0f, 0, 0));

    return wheelMat;
}

glm::mat4 Vehicle::getWheelFRMat() const
{
    glm::mat4 wheelMat =
        bodyMat *
        glm::translate(glm::mat4(1.0f), glm::vec3(-wheelOffset.x, wheelOffset.y, wheelOffset.z)) /*Offset*/ *
        glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0, 1, 0)) /*Flip*/;

    // Steer
    wheelMat = glm::rotate(wheelMat, steeringAngleRad, glm::vec3(0, 1.0f, 0));

    // Camber
    wheelMat = glm::rotate(wheelMat, camberRad, glm::vec3(0, 0, 1));

    // Spin
    wheelMat = glm::rotate(wheelMat, wheelSpin, glm::vec3(-1.0f, 0, 0));

    return wheelMat;
}

glm::mat4 Vehicle::getWheelBLMat() const
{
    glm::mat4 wheelMat =
        bodyMat *
        glm::translate(glm::mat4(1.0f), glm::vec3(wheelOffset.x, wheelOffset.y, -wheelOffset.z)) /*Offset*/;

    // Camber
    wheelMat = glm::rotate(wheelMat, camberRad, glm::vec3(0, 0, 1));

    // Spin
    wheelMat = glm::rotate(wheelMat, wheelSpin, glm::vec3(1.0f, 0, 0));

    return wheelMat;
}

glm::mat4 Vehicle::getWheelBRMat() const
{
    glm::mat4 wheelMat =
        bodyMat *
        glm::translate(glm::mat4(1.0f), glm::vec3(-wheelOffset.x, wheelOffset.y, -wheelOffset.z)) /*Offset*/ *
        glm::rotate(glm::mat4(1.0f), PI, glm::vec3(0, 1, 0)) /*Flip*/;

    // Camber
    wheelMat = glm::rotate(wheelMat, camberRad, glm::vec3(0, 0, 1));

    // Spin
    wheelMat = glm::rotate(wheelMat, wheelSpin, glm::vec3(-1.0f, 0, 0));

    return wheelMat;
}

void Vehicle::tick(VehicleInputState vis, Environment environment, float surfaceFriction, ve_time_t deltaTime)
{
    dt = deltaTime;

    this->vis = vis;

    steer();

    stallAssist();

    calcForces(environment, surfaceFriction);

    calcRpm();

    updateTransmission();

    wheelSpin = std::fmod(wheelSpin + speedMps * dt / wheelRadiusM, 2.0f * PI);

    updateTransform();

    bodyMat = transformToMat(transform);

    // std::cout << std::round(speedMps * 3.6f) << " km/h, " << std::round(rpm) << " rpm, " << gear << " gear" << std::endl;
}