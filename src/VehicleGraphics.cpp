// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

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
    position.z += speedMps * dt;

    bodyMat = glm::translate(bodyMat, glm::vec3(position.x, position.y, position.z));
}

void Vehicle::offsetWheels()
{
    // Temporary
    Position3 wheelOffset = {2.0f, 0.4f, 1.8f};

    wheelFLMat = glm::translate(wheelFLMat, glm::vec3(wheelOffset.x / 2, bodyMat[3][1] + wheelOffset.y, bodyMat[3][2] + wheelOffset.z));
    wheelFRMat = glm::translate(wheelFRMat, glm::vec3(-wheelOffset.x / 2, bodyMat[3][1] + wheelOffset.y, bodyMat[3][2] + wheelOffset.z));
    wheelBLMat = glm::translate(wheelBLMat, glm::vec3(wheelOffset.x / 2, bodyMat[3][1] + wheelOffset.y, bodyMat[3][2] - wheelOffset.z));
    wheelBRMat = glm::translate(wheelBRMat, glm::vec3(-wheelOffset.x / 2, bodyMat[3][1] + wheelOffset.y, bodyMat[3][2] - wheelOffset.z));

    wheelFRMat = glm::rotate(wheelFRMat, glm::radians(180.0f), glm::vec3(0, 1.0f, 0));
    wheelBRMat = glm::rotate(wheelBRMat, glm::radians(180.0f), glm::vec3(0, 1.0f, 0));
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

    wheelFLMat = glm::rotate(wheelFLMat, wheelSpin, glm::vec3(1.0f, 0, 0));
    wheelFRMat = glm::rotate(wheelFRMat, wheelSpin, glm::vec3(1.0f, 0, 0));
    wheelBLMat = glm::rotate(wheelBLMat, wheelSpin, glm::vec3(1.0f, 0, 0));
    wheelBRMat = glm::rotate(wheelBRMat, wheelSpin, glm::vec3(1.0f, 0, 0));
}

void Vehicle::updateGraphics()
{
    resetMatrices();
    move();
    offsetWheels();
    steerWheels();
    spinWheels();
}