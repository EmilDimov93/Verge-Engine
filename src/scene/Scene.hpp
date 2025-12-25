// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../rendering/Mesh.hpp"
#include "../rendering/Camera.hpp"
#include "../vehicle/Vehicle.hpp"
#include "Trigger.hpp"

class Scene
{
public:
    std::vector<Mesh> meshes;
    std::vector<Vehicle> vehicles;

    Scene(VulkanContext newVulkanContext, float newFov, float newAspectRatio, float newZNear, float newZFar);

    uint32_t addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info);

    void loadFile(const std::string& filename);

    VulkanContext vulkanContext;

    void updateModel(int modelId, glm::mat4 newModel);

    void tick(ve_time dt);

    void setCameraFollowVehicle(uint32_t vehicleIndex);
    void unsetCameraFollowVehicle(uint32_t vehicleIndex);

    ~Scene();

private:
    bool isCameraFollowingVehicle;
    uint32_t cameraFollowedVehicleIndex;

    void cameraFollowVehicle(ve_time dt);
};