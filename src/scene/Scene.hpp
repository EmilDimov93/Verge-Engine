// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "actors/Ground.hpp"
#include "actors/Vehicle.hpp"
#include "actors/Prop.hpp"
#include "actors/Trigger.hpp"

#include "Camera.hpp"
#include "Environment.hpp"

#include "../shared/Mesh.hpp"

class Scene
{
public:
    Scene(ve_color_t backgroundColor, const VE_STRUCT_CAMERA_CREATE_INFO &cameraInfo);

    DrawData getDrawData();

    MeshId loadFile(const std::string &filePath);

    void addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info, Transform transform = {});
    void addProp(MeshId meshId, Transform transform);
    void addTrigger(int32_t id, const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info, Transform transform = {});

    void setMatrix(MeshInstanceId meshInstanceId, glm::mat4 newModel);

    void tick(ve_time_t dt);

    void setCameraFollowVehicle(uint32_t vehicleIndex);
    void unsetCameraFollowVehicle();
    void setCameraFollowDistance(float distance);
    void setCameraFollowHeight(float height);
    void setCameraFollowYawDelay(float yawDelay);

    uint32_t addSurface(const VE_STRUCT_SURFACE_CREATE_INFO &info);
    void buildGroundMesh(Size2 size, Transform transform = {});

    void setAirDensity(float airDensity);
    void setGravity(float gravity);
    void setBackgroundColor(ve_color_t backgroundColor);

    ~Scene();

private:
    ve_time_t dt;

    // Meshes
    std::vector<Mesh> meshes;
    std::vector<MeshInstance> meshInstances;
    uint64_t lastMeshId = 0;
    uint64_t lastMeshInstanceId = 0;

    MeshId getNextMeshId();
    MeshInstanceId getNextMeshInstanceId();

    Ground ground;
    std::vector<Surface> surfaces;

    std::vector<Vehicle> vehicles;
    std::vector<Prop> props;
    std::vector<Trigger> triggers;

    // Environment
    Environment environment;

    // Camera
    Camera camera;
    bool isCameraFollowingVehicle;
    uint32_t cameraFollowedVehicleIndex;
    float cameraFollowDistance = 10.0f;
    float cameraFollowHeight = 3.0f;
    float cameraFollowYawDelay = 0.01f;

    MeshId loadOBJ(const std::string &filePath);
    MeshId loadFBX(const std::string &filePath);
    MeshId loadGLB(const std::string &filePath);
    MeshId loadGLTF(const std::string &filePath);

    void makeExampleGround();

    MeshInstanceId addMeshInstance(int64_t meshId);

    void cameraFollowVehicle();
};