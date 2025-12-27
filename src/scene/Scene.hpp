// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../mesh/Mesh.hpp"
#include "camera/Camera.hpp"

#include "../actors/vehicle/Vehicle.hpp"
#include "../actors/prop/Prop.hpp"
#include "../actors/trigger/Trigger.hpp"

class Scene
{
public:
    std::vector<Mesh> meshes;

    std::vector<Vehicle> vehicles;
    std::vector<Prop> props;
    std::vector<Trigger> triggers;

    Scene(VulkanContext newVulkanContext, float newFov, float newAspectRatio, float newZNear, float newZFar);

    uint32_t loadFile(const std::string& filename);

    uint32_t addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info);
    uint32_t addProp(uint32_t meshIndex, Position3 position, Rotation3 rotation);
    uint32_t addTrigger(uint32_t id, Position3 position, const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info);

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