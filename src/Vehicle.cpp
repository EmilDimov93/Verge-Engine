// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"
#include "Log.hpp"

#define PI 3.1415926f

#define TORQUE_CONVERSION_CONSTANT 5252
#define GRAVITY_CONSTANT 9.81f

#define AIR_DENSITY 1.225f

#define BASELINE_TORQUE_FACTOR 0.9f
#define SURFACE_ROLLING_COEFFICIENT 0.015f

void Vehicle::init(const VE_STRUCT_VEHICLE_CREATE_INFO &info)
{
    if (info.bodyMeshIndex != -1)
    {
        bodyMeshIndex = info.bodyMeshIndex;
    }
    else
    {
        Log::add('A', 100);
    }
    if (info.wheelFLMeshIndex != -1 && info.wheelFRMeshIndex != -1 && info.wheelBLMeshIndex != -1 && info.wheelBRMeshIndex != -1)
    {
        wheelFLMeshIndex = info.wheelFLMeshIndex;
        wheelFRMeshIndex = info.wheelFRMeshIndex;
        wheelBLMeshIndex = info.wheelBLMeshIndex;
        wheelBRMeshIndex = info.wheelBRMeshIndex;
    }
    else
    {
        Log::add('A', 101);
    }

    bodyMat = glm::mat4(1.0f);
    wheelFLMat = glm::mat4(1.0f);
    wheelFRMat = glm::mat4(1.0f);
    wheelBLMat = glm::mat4(1.0f);
    wheelBRMat = glm::mat4(1.0f);

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

    transmissionType = info.transmissionType;

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
    shiftUpKey = info.shiftUpKey;
    shiftDownKey = info.shiftDownKey;

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

    if (rpm >= maxRpm)
    {
        wheelTorque = 0;
    }

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

void Vehicle::resetMatrices()
{
    bodyMat = glm::mat4(1.0f);
    wheelFLMat = glm::mat4(1.0f);
    wheelFRMat = glm::mat4(1.0f);
    wheelBLMat = glm::mat4(1.0f);
    wheelBRMat = glm::mat4(1.0f);
}

void Vehicle::move()
{
    static float z = 100.0f;

    z -= speedMps * dt;

    position = {0, 0, z};

    bodyMat = glm::translate(bodyMat, glm::vec3(0, 0, z));
}

void Vehicle::offsetWheels()
{
    // Temporary
    Position2 wheelOffset = {2.5f, 2.0f};

    wheelFLMat = glm::translate(wheelFLMat, glm::vec3(wheelOffset.x / 2, 0, bodyMat[3][2] - wheelOffset.y));
    wheelFRMat = glm::translate(wheelFRMat, glm::vec3(-wheelOffset.x / 2, 0, bodyMat[3][2] - wheelOffset.y));
    wheelBLMat = glm::translate(wheelBLMat, glm::vec3(wheelOffset.x / 2, 0, bodyMat[3][2] + wheelOffset.y));
    wheelBRMat = glm::translate(wheelBRMat, glm::vec3(-wheelOffset.x / 2, 0, bodyMat[3][2] + wheelOffset.y));
}

void Vehicle::steerWheels()
{
    wheelFLMat = glm::rotate(wheelFLMat, steeringAngleRad, glm::vec3(0, 1.0f, 0));
    wheelFRMat = glm::rotate(wheelFRMat, steeringAngleRad, glm::vec3(0, 1.0f, 0));
}

void Vehicle::spinWheels()
{
    static float wheelSpin = 0;

    wheelSpin += speedMps / wheelRadiusM * dt;

    wheelFLMat = glm::rotate(wheelFLMat, wheelSpin, glm::vec3(-1.0f, 0, 0));
    wheelFRMat = glm::rotate(wheelFRMat, wheelSpin, glm::vec3(-1.0f, 0, 0));
    wheelBLMat = glm::rotate(wheelBLMat, wheelSpin, glm::vec3(-1.0f, 0, 0));
    wheelBRMat = glm::rotate(wheelBRMat, wheelSpin, glm::vec3(-1.0f, 0, 0));
}

void Vehicle::update(ve_time deltaTime)
{
    dt = deltaTime;

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

    resetMatrices();
    move();
    offsetWheels();
    steerWheels();
    spinWheels();

    // std::cout << "Speed: " << speedMps * 3.6f << " km/h, RPM: " << std::round(rpm) << " , Gear: " << gear << std::endl;
}