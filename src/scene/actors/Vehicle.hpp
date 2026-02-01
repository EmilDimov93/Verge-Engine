// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../Environment.hpp"

#include "../Controller.hpp"

#include "../../shared/definitions.hpp"

#include <vector>

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

    uint32_t peakTorqueNm = 300;

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

    std::string engineAudioFileName;
};

struct WheelState
{
    float grip = 1.0f;
    float suspension = 0;
    float spin = 0;

    // wear, temperature
};

class Vehicle
{
public:
    Vehicle(VehicleHandle handle, Transform transform, const VE_STRUCT_VEHICLE_CREATE_INFO &info, MeshInstanceHandle bodyMeshInstanceHandle, MeshInstanceHandle wheelFLMeshInstanceHandle, MeshInstanceHandle wheelFRMeshInstanceHandle, MeshInstanceHandle wheelBLMeshInstanceHandle, MeshInstanceHandle wheelBRMeshInstanceHandle);

    void tick(VehicleInputState vis, Environment environment, float surfaceFriction, ve_time_t deltaTime);

    // Temporary(testing)
    void setHeight(float h) { transform.position.y = h; }

    // Temporarily public
    WheelState flState;
    WheelState frState;
    WheelState blState;
    WheelState brState;

    // Temporarily public
    // Points of interest for collision checks(local)
    glm::vec3 flPOI;
    glm::vec3 frPOI;
    glm::vec3 blPOI;
    glm::vec3 brPOI;

    glm::mat4 transformMat;

    glm::vec3 getFLPOIWorld()
    {
        return transformMat * glm::vec4(flPOI, 1.0f);
    }

    glm::vec3 getFRPOIWorld()
    {
        return transformMat * glm::vec4(frPOI, 1.0f);
    }

    glm::vec3 getBLPOIWorld()
    {
        return transformMat * glm::vec4(blPOI, 1.0f);
    }

    glm::vec3 getBRPOIWorld()
    {
        return transformMat * glm::vec4(brPOI, 1.0f);
    }

    glm::vec3 getFLPOILocal()
    {
        return flPOI;
    }

    glm::vec3 getFRPOILocal()
    {
        return frPOI;
    }

    glm::vec3 getBLPOILocal()
    {
        return blPOI;
    }

    glm::vec3 getBRPOILocal()
    {
        return brPOI;
    }

    float getMaxClimb()
    {
        // Temporary hardcode. Should be wheelRadius?
        return 0.5f;
    }

    void collideVelocityVector(glm::vec3 localPOI);

    void updateTransform();

private:
    void stallAssist();
    void cruiseControl();
    float getTorque();
    float calcFDriveMag();
    void calcForces(const Environment &environment);
    void steer();
    void shiftUp();
    void shiftDown();
    void updateTransmission();

    const VehicleHandle handle;

    MeshInstanceHandle bodyMeshInstanceHandle;
    MeshInstanceHandle wheelFLMeshInstanceHandle;
    MeshInstanceHandle wheelFRMeshInstanceHandle;
    MeshInstanceHandle wheelBLMeshInstanceHandle;
    MeshInstanceHandle wheelBRMeshInstanceHandle;

    Position3 wheelOffset;

    uint32_t peakTorqueNm;
    float weightKg;
    uint32_t gearCount;
    uint32_t maxRpm;
    uint32_t idleRpm;
    VETransmissionType transmissionType;
    float brakingForce;
    std::vector<float> gearRatios;
    float finalDriveRatio;
    float drivetrainEfficiency;
    float wheelRadiusM;
    float dragCoeff;
    float frontalAreaM2;
    float maxSteeringAngleRad;
    float tireGrip;
    float camberRad;

    // Runtime
    VehicleInputState vis;

    Transform transform;
    glm::vec3 velocityMps = glm::vec3(0.0f);

    float speedMps = 0.0f;
    float forwardSpeedMps = 0.0f;
    uint32_t gear = 1;
    float rpm = 0.0f;

    float steeringAngleRad = 0.0f;
    float wheelSpin = 0.0f;
    float yawRateRadps = 0.0f;

    float cruiseControlTargetMps = 0;

    glm::mat4 bodyMat;

    ve_time_t dt;

public:
    // Getters
    VehicleHandle getHandle() const { return handle; };

    glm::mat4 getBodyMat() const { return bodyMat; };
    glm::mat4 getWheelFLMat() const;
    glm::mat4 getWheelFRMat() const;
    glm::mat4 getWheelBLMat() const;
    glm::mat4 getWheelBRMat() const;

    MeshInstanceHandle getBodyMeshInstanceHandle() const { return bodyMeshInstanceHandle; };
    MeshInstanceHandle getWheelFLMeshInstanceHandle() const { return wheelFLMeshInstanceHandle; };
    MeshInstanceHandle getWheelFRMeshInstanceHandle() const { return wheelFRMeshInstanceHandle; };
    MeshInstanceHandle getWheelBLMeshInstanceHandle() const { return wheelBLMeshInstanceHandle; };
    MeshInstanceHandle getWheelBRMeshInstanceHandle() const { return wheelBRMeshInstanceHandle; };
    Position3 getWheelOffset() const { return wheelOffset; }
    uint32_t getPeakTorqueNm() const { return peakTorqueNm; }
    float getWeightKg() const { return weightKg; }
    uint32_t getGearCount() const { return gearCount; }
    uint32_t getMaxRpm() const { return maxRpm; }
    uint32_t getIdleRpm() const { return idleRpm; }
    VETransmissionType getTransmissionType() const { return transmissionType; }
    float getBrakingForce() const { return brakingForce; }
    float getGearRatio(uint32_t gearIndex) const
    {
        return gearIndex < gearRatios.size() ? gearRatios[gearIndex] : 0.0f;
    }
    float getFinalDriveRatio() const { return finalDriveRatio; }
    float getDrivetrainEfficiency() const { return drivetrainEfficiency; }
    float getWheelRadius() const { return wheelRadiusM; }
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
    void setPeakTorqueNm(uint32_t value) { peakTorqueNm = value; }
    void setWeightKg(float value) { weightKg = value; }
    void setGearCount(uint32_t value) { gearCount = value; }
    void setMaxRpm(uint32_t value) { maxRpm = value; }
    void setIdleRpm(uint32_t value) { idleRpm = value; }
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
    void setCruiseControlTargetMps(float value) { cruiseControlTargetMps = value; }
    void setCruiseControlTargetKmph(float value) { cruiseControlTargetMps = value / 3.6f; }

    // Other
    void start();
    void stopCruiseControl() { cruiseControlTargetMps = 0; }
};