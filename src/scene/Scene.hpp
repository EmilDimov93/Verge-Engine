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
    std::vector<MeshInstance> meshInstances;

    std::vector<Vehicle> vehicles;
    std::vector<Prop> props;
    std::vector<Trigger> triggers;

    Scene(VulkanContext newVulkanContext, float newFov, float newAspectRatio, float newZNear, float newZFar);

    uint32_t loadFile(const std::string &filePath);

    uint32_t addVehicle(Transform transform, const VE_STRUCT_VEHICLE_CREATE_INFO &info);
    uint32_t addProp(uint32_t meshIndex, Transform transform);
    uint32_t addTrigger(uint32_t id, Transform transform, const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info);

    VulkanContext vulkanContext;

    void setMatrix(int meshInstanceIndex, glm::mat4 newModel);

    void tick(ve_time dt);

    void setCameraFollowVehicle(uint32_t vehicleIndex);
    void unsetCameraFollowVehicle();
    void setCameraFollowDistance(float distance);
    void setCameraFollowHeight(float height);
    void setCameraFollowYawDelay(float yawDelay);

    ~Scene();

private:
    bool isCameraFollowingVehicle;
    uint32_t cameraFollowedVehicleIndex;
    float cameraFollowDistance = 10.0f;
    float cameraFollowHeight = 3.0f;
    float cameraFollowYawDelay = 0.01f;

    uint32_t loadOBJ(const std::string &filePath);
    uint32_t loadFBX(const std::string &filePath);
    uint32_t loadGLB(const std::string &filePath);
    uint32_t loadGLTF(const std::string &filePath);

    uint32_t addMeshInstance(uint32_t meshIndex);

    void cameraFollowVehicle(ve_time dt);
};