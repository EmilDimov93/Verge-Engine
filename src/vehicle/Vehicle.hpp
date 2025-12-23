// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../definitions.hpp"
#include "../system/Input.hpp"
#include "../rendering/Mesh.hpp"

enum VEPowerUnit
{
    VE_POWER_UNIT_KILOWATTS,
    VE_POWER_UNIT_HORSEPOWER
};

enum VETransmissionType
{
    VE_TRANSMISSION_TYPE_MANUAL,
    VE_TRANSMISSION_TYPE_AUTOMATIC
};

struct VE_STRUCT_VEHICLE_CREATE_INFO
{
    int32_t bodyMeshIndex = -1;
    int32_t wheelFLMeshIndex = -1;
    int32_t wheelFRMeshIndex = -1;
    int32_t wheelBLMeshIndex = -1;
    int32_t wheelBRMeshIndex = -1;

    uint32_t power = 100;
    VEPowerUnit powerUnit = VE_POWER_UNIT_KILOWATTS;

    float weightKg = 1200.f;
    uint32_t gearCount = 5;
    uint32_t maxRpm = 6000;
    float brakingForce = 1.0f;
    VETransmissionType transmissionType = VE_TRANSMISSION_TYPE_AUTOMATIC;

    VEKey accelerateKey = VE_KEY_UNKNOWN;
    VEKey brakeKey = VE_KEY_UNKNOWN;
    VEKey turnLeftKey = VE_KEY_UNKNOWN;
    VEKey turnRightKey = VE_KEY_UNKNOWN;
    VEKey shiftUpKey = VE_KEY_UNKNOWN;
    VEKey shiftDownKey = VE_KEY_UNKNOWN;

    float *pGearRatios = nullptr;
    float finalDriveRatio = 3.42f;
    float drivetrainEfficiency = 0.85f;
    float wheelRadiusM = 0.3f;
    float dragCoeff = 0.31f;
    float frontalAreaM2 = -1;
    float maxSteeringAngleRad = 0.55f;
    float idleRpm = 800.f;
    float camber = 0;
};

class Vehicle
{
public:
    // Temporarily public
    uint32_t bodyMeshIndex;
    int32_t wheelFLMeshIndex;
    int32_t wheelFRMeshIndex;
    int32_t wheelBLMeshIndex;
    int32_t wheelBRMeshIndex;

    glm::mat4 bodyMat;
    glm::mat4 wheelFLMat, wheelFRMat, wheelBLMat, wheelBRMat;

    void init(const VE_STRUCT_VEHICLE_CREATE_INFO &info);

    void tick(ve_time deltaTime);

    void calculatePhysics();
    void updateGraphics();

    uint32_t getPowerKw() const { return powerKw; }
    uint32_t getPowerHp() const{ return static_cast<uint32_t>(powerKw * 1.341022f); }

    float getWeightKg() const { return weightKg; }
    uint32_t getGearCount() const { return gearCount; }
    uint32_t getMaxRpm() const { return maxRpm; }
    VETransmissionType getTransmissionType() const { return transmissionType; }
    float getBrakingForce() const { return brakingForce; }

    float getGearRatio(uint32_t gearIndex) const
    {
        return gearIndex < gearRatios.size() ? gearRatios[gearIndex] : 0.0f;
    }

    float getFinalDriveRatio() const { return finalDriveRatio; }
    float getDrivetrainEfficiency() const { return drivetrainEfficiency; }
    float getWheelRadius() const { return wheelRadiusM; }
    float getIdleRpm() const { return idleRpm; }
    float getDragCoeff() const { return dragCoeff; }
    float getFrontalArea() const { return frontalAreaM2; }
    float getMaxSteeringAngleRad() const { return maxSteeringAngleRad; }

    float getMaxSteeringAngleDeg() const{ return maxSteeringAngleRad * 57.2957795f; }

    const Position3 &getPosition() const { return position; }
    const Rotation3 &getRotation() const { return rotation; }

    float getSpeedMps() const { return speedMps; }
    float getSpeedKmph() const { return speedMps * 3.6f; }

    float getSteeringAngleRad() const { return steeringAngleRad; }
    float getSteeringAngleDeg() const { return steeringAngleRad * 57.2957795f; }

    uint32_t getGear() const { return gear; }
    float getRpm() const { return rpm; }
    float getTireGrip() const { return tireGrip; }
    float getClutchState() const { return clutchState; }

    void setPowerKw(uint32_t value) { powerKw = value; }
    void setPowerHp(uint32_t value){ powerKw = static_cast<uint32_t>(value / 1.341022f); }

    void setWeightKg(float value) { weightKg = value; }
    void setGearCount(uint32_t value) { gearCount = value; }
    void setMaxRpm(uint32_t value) { maxRpm = value; }
    void setTransmissionType( VETransmissionType value ) { transmissionType = value; }
    void setBrakingForce(float value) { brakingForce = value; }

    void setGearRatio(uint32_t gearIndex, float value)
    {
        if (gearIndex < gearRatios.size() && gearIndex >= 0)
            gearRatios[gearIndex] = value;
    }

    void setFinalDriveRatio(float value) { finalDriveRatio = value; }
    void setDrivetrainEfficiency(float value) { drivetrainEfficiency = value; }
    void setWheelRadius(float value) { wheelRadiusM = value; }
    void setIdleRpm(float value) { idleRpm = value; }
    void setDragCoeff(float value) { dragCoeff = value; }
    void setFrontalArea(float value) { frontalAreaM2 = value; }

    void setMaxSteeringAngleRad(float value) { maxSteeringAngleRad = value; }
    void setMaxSteeringAngleDeg(float deg){ maxSteeringAngleRad = deg * 0.0174532925f; }

    void setPosition(const Position3 &value) { position = value; }
    void setRotation(const Rotation3 &value) { rotation = value; }

    void setSpeedMps(float value) { speedMps = value; }

    void setSteeringAngleRad(float value) { steeringAngleRad = value; }
    void setSteeringAngleDeg(float value) { steeringAngleRad = value * 0.0174532925f; }

    void setGear(uint32_t value) { gear = value; }
    void setRpm(float value) { rpm = value; }
    void setTireGrip(float value) { tireGrip = value; }
    void setClutchState(float value) { clutchState = value; }

private:
    void calcSpeed();
    void handleInput();
    void turnLeft();
    void turnRight();
    void shiftUp();
    void shiftDown();
    void updateTransmission();

    void resetMatrices();
    void move();
    void offsetWheels();
    void steerWheels();
    void spinWheels();

    // Position3 wheelOffset;

    uint32_t powerKw;
    float weightKg;
    uint32_t gearCount;
    uint32_t maxRpm;
    VETransmissionType transmissionType;
    float brakingForce;

    VEKey accelerateKey;
    VEKey brakeKey;
    VEKey turnLeftKey;
    VEKey turnRightKey;
    VEKey shiftUpKey;
    VEKey shiftDownKey;

    std::vector<float> gearRatios;
    float finalDriveRatio;
    float drivetrainEfficiency;
    float wheelRadiusM;
    float idleRpm;
    float dragCoeff;
    float frontalAreaM2;
    float maxSteeringAngleRad;

    float camber;

    // Runtime
    Position3 position = {0, 0, -100.0f}; // Temporary default
    Rotation3 rotation;
    Rotation3 moveDirection;
    float speedMps;
    float steeringAngleRad;
    uint32_t gear;
    float rpm;
    float tireGrip;
    float clutchState;
    float throttleState;
    float brakeState;
    ve_time dt;
};