// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../mesh/Mesh.hpp"
#include "camera/Camera.hpp"
#include "environment/Environment.hpp"

#include "../actors/ground/Ground.hpp"

#include "../actors/vehicle/Vehicle.hpp"
#include "../actors/prop/Prop.hpp"
#include "../actors/trigger/Trigger.hpp"

class Scene
{
public:
    Scene(VulkanContext vulkanContext, const VE_STRUCT_CAMERA_CREATE_INFO& cameraInfo);

    DrawData getDrawData();

    uint32_t loadFile(const std::string &filePath);

    uint32_t addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info, Transform transform = {});
    uint32_t addProp(int32_t meshIndex, Transform transform);
    uint32_t addTrigger(int32_t id, const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info, Transform transform = {});

    VulkanContext vulkanContext;

    void setMatrix(int meshInstanceIndex, glm::mat4 newModel);

    void tick(ve_time_t dt);

    void setCameraFollowVehicle(uint32_t vehicleIndex);
    void unsetCameraFollowVehicle();
    void setCameraFollowDistance(float distance);
    void setCameraFollowHeight(float height);
    void setCameraFollowYawDelay(float yawDelay);

    uint32_t addSurface(const VE_STRUCT_SURFACE_CREATE_INFO &info);
    void buildGroundMesh(Size2 size, Transform transform = {});

    ~Scene();

private:
    ve_time_t dt;

    std::vector<Mesh> meshes;
    std::vector<MeshInstance> meshInstances;

    Ground ground;
    std::vector<Surface> surfaces;

    std::vector<Vehicle> vehicles;
    std::vector<Prop> props;
    std::vector<Trigger> triggers;

    // Scenery
    Environment environment;

    // Camera
    Camera camera;

    bool isCameraFollowingVehicle;
    uint32_t cameraFollowedVehicleIndex;
    float cameraFollowDistance = 10.0f;
    float cameraFollowHeight = 3.0f;
    float cameraFollowYawDelay = 0.01f;

    uint32_t loadOBJ(const std::string &filePath);
    uint32_t loadFBX(const std::string &filePath);
    uint32_t loadGLB(const std::string &filePath);
    uint32_t loadGLTF(const std::string &filePath);

    void makeExampleGround();

    uint32_t addMeshInstance(int32_t meshIndex);

    void cameraFollowVehicle();
};