// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../Environment.hpp"

#include "../Controller.hpp"

#include "../../shared/AudioData.hpp"
#include "../../shared/definitions.hpp"

#include <vector>
#include <array>

namespace VE
{

    constexpr float RPM_TO_RADPS_CONVERSION_FACTOR = 2.0f * PI / 60.0f;

    enum TransmissionType
    {
        TRANSMISSION_TYPE_AUTOMATIC,
        TRANSMISSION_TYPE_MANUAL,
        TRANSMISSION_TYPE_MANUAL_WITH_CLUTCH
    };

    enum DrivetrainType
    {
        DRIVETRAIN_TYPE_AWD,
        DRIVETRAIN_TYPE_FWD,
        DRIVETRAIN_TYPE_RWD
    };

    struct VehicleCreateInfo
    {
        ModelHandle bodyModelHandle = INVALID_MODEL_HANDLE;
        ModelHandle wheelModelHandle = INVALID_MODEL_HANDLE;

        Position3 wheelOffset{0};

        uint32_t peakTorqueNm = 300;

        float weightKg = 1200.f;
        uint32_t maxRpm = 6000;
        float brakingForceN = 15000.0f;
        TransmissionType transmissionType = TRANSMISSION_TYPE_AUTOMATIC;

        std::vector<float> gearRatios;
        float reverseGearRatio = 3.5f;
        float finalDriveRatio = 4.0f;
        float drivetrainEfficiency = 0.85f;
        float wheelRadiusM = 0.3f;
        float dragCoeff = 0.31f;
        float frontalAreaM2 = 2.0f;
        float maxSteeringAngleRad = 0.55f;
        float tireGrip = 1.0f;
        float idleRpm = 800.f;
        float camberRad = 0;

        DrivetrainType drivetrainType = DRIVETRAIN_TYPE_AWD;

        std::string engineAudioFileName;
        std::vector<EngineAudioFileRequest> layeredEngineAudioFiles;
    };

    enum Wheel
    {
        WHEEL_FRONT_LEFT = 0,
        WHEEL_FRONT_RIGHT = 1,
        WHEEL_BACK_LEFT = 2,
        WHEEL_BACK_RIGHT = 3,
        WHEEL_COUNT
    };

    class Vehicle
    {
    public:
        Vehicle(VehicleHandle handle, Transform transform, const VehicleCreateInfo &info, ModelInstanceHandle bodyModelInstanceHandle, ModelInstanceHandle wheelFLModelInstanceHandle, ModelInstanceHandle wheelFRModelInstanceHandle, ModelInstanceHandle wheelBLModelInstanceHandle, ModelInstanceHandle wheelBRModelInstanceHandle);

        void tick(VehicleInputState vis, Environment environment, float surfaceFriction, milliseconds_t deltaTime);

        // Temporary(testing)
        void setHeight(float h) { transform.position.y = h; }

        static constexpr size_t CollisionPointCount = 4;
        std::array<glm::vec3, CollisionPointCount> collisionPoints;

        [[nodiscard]] glm::vec3 getCollisionPointWorld(uint32_t index)
        {
            assert(index < collisionPoints.size());
            return bodyMat * glm::vec4(collisionPoints[index], 1.0f);
        }

        [[nodiscard]] glm::vec3 getCollisionPointLocal(uint32_t index)
        {
            assert(index < collisionPoints.size());
            return collisionPoints[index];
        }

        [[nodiscard]] float getMaxClimb()
        {
            return wheelRadiusM;
        }

        void collideVelocityVector(glm::vec3 localCollisionPoint);

        void updateTransform();

    private:
        void stallAssist();
        void cruiseControl();
        [[nodiscard]] float getTorque();
        [[nodiscard]] float calcFDriveMag();
        void calcForces(const Environment &environment);
        void steer();
        void shiftUp();
        void shiftDown();
        void updateTransmission();
        void calcTireTemperatures(const Environment &environment);

        void activateStarter();

        struct WheelState
        {
            float grip = 1.0f;
            float suspension = 0.0f;
            float rpm = 0.0f;
            float spin = 0.0f;

            float temperatureK = 0.0f;

            float wear = 0.0f; // Not implemented
        };

        VehicleHandle handle;

        ModelInstanceHandle bodyModelInstanceHandle;
        std::array<ModelInstanceHandle, WHEEL_COUNT> wheelModelInstanceHandles;

        Position3 wheelOffset;

        uint32_t peakTorqueNm;
        float weightKg;
        uint32_t gearCount;
        uint32_t maxRpm;
        uint32_t idleRpm;
        TransmissionType transmissionType;
        float brakingForceN;
        std::vector<float> gearRatios;
        float finalDriveRatio;
        float drivetrainEfficiency;
        float wheelRadiusM;
        float dragCoeff;
        float frontalAreaM2;
        float maxSteeringAngleRad;
        float tireGrip;
        float camberRad;

        DrivetrainType drivetrainType;

        // Runtime
        VehicleInputState vis;

        Transform transform;
        glm::vec3 velocityMps = glm::vec3(0.0f);

        float speedMps = 0.0f;
        float forwardSpeedMps = 0.0f;
        uint32_t gear = 1;
        bool isNeutral = true;
        float rpm = 0.0f;
        float wheelRpm = 0.0f;

        float steeringAngleRad = 0.0f;
        float yawRateRadps = 0.0f;

        float cruiseControlTargetMps = 0;

        std::array<WheelState, WHEEL_COUNT> wheelStates;

        glm::mat4 bodyMat;

        milliseconds_t dt;

        // Debug
        void printState() const;
        void printVIS() const;

    public:
        // Getters
        VehicleHandle getHandle() const { return handle; };

        glm::mat4 getBodyMat() const { return bodyMat; };
        glm::mat4 getWheelMat(Wheel wheel) const;

        [[nodiscard]] ModelInstanceHandle getBodyModelInstanceHandle() const { return bodyModelInstanceHandle; }
        [[nodiscard]] ModelInstanceHandle getWheelModelInstanceHandle(Wheel wheel) const { return wheelModelInstanceHandles[wheel]; }
        [[nodiscard]] Position3 getWheelOffset() const { return wheelOffset; }
        [[nodiscard]] uint32_t getPeakTorqueNm() const { return peakTorqueNm; }
        [[nodiscard]] float getWeightKg() const { return weightKg; }
        [[nodiscard]] uint32_t getGearCount() const { return gearCount; }
        [[nodiscard]] uint32_t getMaxRpm() const { return maxRpm; }
        [[nodiscard]] uint32_t getIdleRpm() const { return idleRpm; }
        [[nodiscard]] TransmissionType getTransmissionType() const { return transmissionType; }
        [[nodiscard]] float getBrakingForceN() const { return brakingForceN; }
        [[nodiscard]] float getGearRatio(uint32_t gear) const { return gear < gearRatios.size() ? gearRatios[gear] : 0.0f; }
        [[nodiscard]] float getFinalDriveRatio() const { return finalDriveRatio; }
        [[nodiscard]] float getDrivetrainEfficiency() const { return drivetrainEfficiency; }
        [[nodiscard]] float getWheelRadius() const { return wheelRadiusM; }
        [[nodiscard]] float getDragCoeff() const { return dragCoeff; }
        [[nodiscard]] float getFrontalArea() const { return frontalAreaM2; }
        [[nodiscard]] float getMaxSteeringAngleRad() const { return maxSteeringAngleRad; }
        [[nodiscard]] float getMaxSteeringAngleDeg() const { return maxSteeringAngleRad * (PI / 180); }
        [[nodiscard]] float getTireGrip() const { return tireGrip; }
        [[nodiscard]] float getCamber() const { return camberRad; }
        [[nodiscard]] const Transform &getTransform() const { return transform; }
        [[nodiscard]] const glm::vec3 &getVelocityVector() const { return velocityMps; }
        [[nodiscard]] float getSpeedMps() const { return speedMps; }
        [[nodiscard]] float getSpeedKmph() const { return speedMps * 3.6f; }
        [[nodiscard]] float getSteeringAngleRad() const { return steeringAngleRad; }
        [[nodiscard]] float getSteeringAngleDeg() const { return steeringAngleRad * (PI / 180); }
        [[nodiscard]] uint32_t getGear() const { return gear; }
        [[nodiscard]] float getRpm() const { return rpm; }

        // Setters
        void setWheelOffset(Position3 value) { wheelOffset = value; }
        void setPeakTorqueNm(uint32_t value) { peakTorqueNm = value; }
        void setWeightKg(float value) { weightKg = value; }
        void setGearCount(uint32_t value) { gearCount = value; }
        void setMaxRpm(uint32_t value) { maxRpm = value; }
        void setIdleRpm(uint32_t value) { idleRpm = value; }
        void setTransmissionType(TransmissionType value) { transmissionType = value; }
        void setBrakingForceN(float value) { brakingForceN = value; }
        void setGearRatio(uint32_t gear, float value)
        {
            if (gear < gearRatios.size())
                gearRatios[gear] = value;
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
        void stopCruiseControl() { cruiseControlTargetMps = 0; }
    };

}