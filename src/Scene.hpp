// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Vehicle.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

class Scene
{
public:
    std::vector<Mesh> meshes;
    std::vector<Vehicle> vehicles;

    Scene(VulkanContext newVulkanContext);

    uint32_t addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info);

    void loadFile(std::string filename, glm::vec3 color);

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