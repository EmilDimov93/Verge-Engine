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
    // Temporary
    moveDirection = rotation;

    double cosPitch = cos(moveDirection.pitch);
    double sinPitch = sin(moveDirection.pitch);
    double cosYaw = cos(moveDirection.yaw);
    double sinYaw = sin(moveDirection.yaw);

    double fx = cosPitch * sinYaw;
    double fy = -sinPitch;
    double fz = cosPitch * cosYaw;

    position.x += fx * speedMps * dt;
    position.y += fy * speedMps * dt;
    position.z += fz * speedMps * dt;

    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

    glm::mat4 R =
        glm::rotate(glm::mat4(1.0f), (float)rotation.yaw, glm::vec3(0, 1, 0)) *
        glm::rotate(glm::mat4(1.0f), (float)rotation.pitch, glm::vec3(1, 0, 0)) *
        glm::rotate(glm::mat4(1.0f), (float)rotation.roll, glm::vec3(0, 0, 1));

    bodyMat = T * R;
}

void Vehicle::offsetWheels()
{
    // Temporary
    Position3 wheelOffset = { 2.0f, 0.4f, 1.8f };

    glm::mat4 flLocal = glm::translate(glm::mat4(1.0f),
        glm::vec3( wheelOffset.x * 0.5f, wheelOffset.y,  wheelOffset.z));

    glm::mat4 frLocal = glm::translate(glm::mat4(1.0f),
        glm::vec3(-wheelOffset.x * 0.5f, wheelOffset.y,  wheelOffset.z));

    glm::mat4 blLocal = glm::translate(glm::mat4(1.0f),
        glm::vec3( wheelOffset.x * 0.5f, wheelOffset.y, -wheelOffset.z));

    glm::mat4 brLocal = glm::translate(glm::mat4(1.0f),
        glm::vec3(-wheelOffset.x * 0.5f, wheelOffset.y, -wheelOffset.z));

    glm::mat4 flip =
        glm::rotate(glm::mat4(1.0f),
            glm::radians(180.0f),
            glm::vec3(0, 1, 0));

    wheelFLMat = bodyMat * flLocal;
    wheelFRMat = bodyMat * frLocal * flip;
    wheelBLMat = bodyMat * blLocal;
    wheelBRMat = bodyMat * brLocal * flip;
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