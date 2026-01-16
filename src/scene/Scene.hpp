// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Player.hpp"

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
    Scene(ve_color_t backgroundColor);

    DrawData getDrawData(PlayerHandle playerHandle);

    MeshHandle loadFile(const std::string &filePath);

    PlayerHandle addPlayer(VehicleHandle vehicleHandle, const PlayerKeybinds &keybinds, const VE_STRUCT_CAMERA_CREATE_INFO &cameraInfo);

    VehicleHandle addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info, Transform transform = {});
    PropHandle addProp(MeshHandle meshHandle, Transform transform);
    TriggerHandle addTrigger(const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info, Transform transform = {});

    void setModelMat(MeshInstanceHandle meshInstanceHandle, glm::mat4 newModel);

    void tick(ve_time_t dt);

    uint32_t addSurface(const VE_STRUCT_SURFACE_CREATE_INFO &info);
    void buildGroundMesh(Size2 size, Transform transform = {});

    void setAirDensity(float airDensity);
    void setGravity(float gravity);
    void setBackgroundColor(ve_color_t backgroundColor);

private:
    ve_time_t dt;

    // Controllers
    std::vector<std::unique_ptr<Controller>> controllers;

    // Meshes
    std::vector<Mesh> meshes;
    std::vector<MeshInstance> meshInstances;

    Ground ground;
    std::vector<Surface> surfaces;

    std::vector<Vehicle> vehicles;
    std::vector<Prop> props;
    std::vector<Trigger> triggers;

    // Environment
    Environment environment;

    MeshHandle loadOBJ(const std::string &filePath);
    MeshHandle loadFBX(const std::string &filePath);
    MeshHandle loadGLB(const std::string &filePath);
    MeshHandle loadGLTF(const std::string &filePath);

    void makeExampleGround();

    MeshInstanceHandle addMeshInstance(MeshHandle meshHandle);
};