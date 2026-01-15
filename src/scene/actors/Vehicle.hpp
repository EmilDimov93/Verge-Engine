// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../Environment.hpp"

#include "../Controller.hpp"

#include "../../shared/definitions.hpp"

#include <vector>

#define KW_TO_HP_CONSTANT 1.341022f

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
    MeshHandle bodyMeshHandle = INVALID_MESH_HANDLE;
    MeshHandle wheelMeshHandle = INVALID_MESH_HANDLE;

    Position3 wheelOffset = {0, 0, 0};

    uint32_t power = 100;
    VEPowerUnit powerUnit = VE_POWER_UNIT_KILOWATTS;

    float weightKg = 1200.f;
    uint32_t gearCount = 5;
    uint32_t maxRpm = 6000;
    float brakingForce = 1.0f;
    VETransmissionType transmissionType = VE_TRANSMISSION_TYPE_AUTOMATIC;

    float *pGearRatios = nullptr;
    float finalDriveRatio = 3.42f;
    float drivetrainEfficiency = 0.85f;
    float wheelRadiusM = 0.3f;
    float dragCoeff = 0.31f;
    float frontalAreaM2 = 2.0f;
    float maxSteeringAngleRad = 0.55f;
    float tireGrip = 1.0f;
    float idleRpm = 800.f;
    float camberRad = 0;
};

class Vehicle
{
public:
    // Temporarily public
    MeshInstanceHandle bodyMeshInstanceHandle;
    MeshInstanceHandle wheelFLMeshInstanceHandle;
    MeshInstanceHandle wheelFRMeshInstanceHandle;
    MeshInstanceHandle wheelBLMeshInstanceHandle;
    MeshInstanceHandle wheelBRMeshInstanceHandle;

    glm::mat4 bodyMat;
    glm::mat4 wheelFLMat, wheelFRMat, wheelBLMat, wheelBRMat;

    Vehicle(VehicleHandle handle, Transform transform, const VE_STRUCT_VEHICLE_CREATE_INFO &info, MeshInstanceHandle bodyMeshInstanceHandle, MeshInstanceHandle wheelFLMeshInstanceHandle, MeshInstanceHandle wheelFRMeshInstanceHandle, MeshInstanceHandle wheelBLMeshInstanceHandle, MeshInstanceHandle wheelBRMeshInstanceHandle);

    void tick(VehicleInputState vis, Environment environment, float surfaceFriction, ve_time_t deltaTime);

    // Getters
    Position3 getWheelOffset() const { return wheelOffset; }
    uint32_t getPowerKw() const { return powerKw; }
    uint32_t getPowerHp() const { return static_cast<uint32_t>(powerKw * KW_TO_HP_CONSTANT); }
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
    float getMaxSteeringAngleDeg() const { return maxSteeringAngleRad * (PI / 180); }
    float getTireGrip() const { return tireGrip; }
    float getCamber() const { return camberRad; }
    const Transform &getTransform() const { return transform; }
    const glm::vec3 &getVelocityVector() const { return velocityMps; }
    float getSpeedMps() const { return speedMps; }
    float getSpeedKmph() const { return speedMps * 3.6f; }
    float getSteeringAngleRad() const { return steeringAngleRad; }
    float getSteeringAngleDeg() const { return steeringAngleRad * (PI / 180); }
    uint32_t getGear() const { return gear; }
    float getRpm() const { return rpm; }

    // Setters
    void setWheelOffset(Position3 value) { wheelOffset = value; }
    void setPowerKw(uint32_t value) { powerKw = value; }
    void setPowerHp(uint32_t value) { powerKw = static_cast<uint32_t>(value / 1.341022f); }
    void setWeightKg(float value) { weightKg = value; }
    void setGearCount(uint32_t value) { gearCount = value; }
    void setMaxRpm(uint32_t value) { maxRpm = value; }
    void setTransmissionType(VETransmissionType value) { transmissionType = value; }
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
    void setMaxSteeringAngleDeg(float deg) { maxSteeringAngleRad = deg * (PI / 180); }
    void setTireGrip(float value) { tireGrip = value; }
    void setCamber(float value) { camberRad = value; }
    void setVelocityVector(const glm::vec3 &value) { velocityMps = value; }
    void setSpeedMps(float value) { speedMps = value; }
    void setSteeringAngleRad(float value) { steeringAngleRad = value; }
    void setSteeringAngleDeg(float value) { steeringAngleRad = value * (PI / 180); }
    void setGear(uint32_t value) { gear = value; }
    void setRpm(float value) { rpm = value; }
    void setVis(VehicleInputState value) { vis = value; }

    // Temporary(testing)
    void setHeight(float h) { transform.position.y = h; }

    VehicleHandle getHandle() const{
        return handle;
    }

private:
    void calculatePhysics(Environment environment, float surfaceFriction);
    void updateGraphics();

    void stallAssist();
    float calcFDriveMag();
    void calcForces(Environment environment, float surfaceFriction);
    void calcRpm();
    void steer(float turningInput);
    void shiftUp();
    void shiftDown();
    void updateTransmission();
    void updateTransform();

    void updateBodyMatrix();
    void updateWheelMatrices();

    const VehicleHandle handle;

    Position3 wheelOffset;

    uint32_t powerKw;
    float weightKg;
    uint32_t gearCount;
    uint32_t maxRpm;
    VETransmissionType transmissionType;
    float brakingForce;

    std::vector<float> gearRatios;
    float finalDriveRatio;
    float drivetrainEfficiency;
    float wheelRadiusM;
    float idleRpm;
    float dragCoeff;
    float frontalAreaM2;
    float maxSteeringAngleRad;
    float tireGrip;
    float camberRad;

    // Runtime
    VehicleInputState vis;

    Transform transform;
    glm::vec3 velocityMps;
    float speedMps;
    float steeringAngleRad;
    uint32_t gear;
    float rpm;
    float wheelSpin;
    ve_time_t dt;
};