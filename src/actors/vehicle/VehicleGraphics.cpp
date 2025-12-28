// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"

void Vehicle::updateBodyMatrix()
{
    bodyMat = transformToMat({position, rotation, {}});
}

void Vehicle::updateWheelMatrices()
{
    // Offset
    glm::mat4 flLocal = glm::translate(glm::mat4(1.0f), glm::vec3(wheelOffset.x, wheelOffset.y, wheelOffset.z));

    glm::mat4 frLocal = glm::translate(glm::mat4(1.0f), glm::vec3(-wheelOffset.x, wheelOffset.y, wheelOffset.z));

    glm::mat4 blLocal = glm::translate(glm::mat4(1.0f), glm::vec3(wheelOffset.x, wheelOffset.y, -wheelOffset.z));

    glm::mat4 brLocal = glm::translate(glm::mat4(1.0f), glm::vec3(-wheelOffset.x, wheelOffset.y, -wheelOffset.z));

    glm::mat4 flip = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));

    wheelFLMat = bodyMat * flLocal;
    wheelFRMat = bodyMat * frLocal * flip;
    wheelBLMat = bodyMat * blLocal;
    wheelBRMat = bodyMat * brLocal * flip;

    // Steer
    wheelFLMat = glm::rotate(wheelFLMat, steeringAngleRad, glm::vec3(0, 1.0f, 0));
    wheelFRMat = glm::rotate(wheelFRMat, steeringAngleRad, glm::vec3(0, 1.0f, 0));

    // Spin
    static float wheelSpin = 0;

    wheelSpin += speedMps * dt / wheelRadiusM;

    wheelFLMat = glm::rotate(wheelFLMat, wheelSpin, glm::vec3(1.0f, 0, 0));
    wheelFRMat = glm::rotate(wheelFRMat, wheelSpin, glm::vec3(1.0f, 0, 0));
    wheelBLMat = glm::rotate(wheelBLMat, wheelSpin, glm::vec3(1.0f, 0, 0));
    wheelBRMat = glm::rotate(wheelBRMat, wheelSpin, glm::vec3(1.0f, 0, 0));
}

void Vehicle::updateGraphics()
{
    updateBodyMatrix();
    updateWheelMatrices();
}