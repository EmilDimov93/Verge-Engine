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

    DrawData getDrawData(PlayerId playerId);

    MeshId loadFile(const std::string &filePath);

    void addPlayer(Player player);

    void addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info, Transform transform = {});
    void addProp(MeshId meshId, Transform transform);
    void addTrigger(TriggerId id, const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info, Transform transform = {});

    void setMatrix(MeshInstanceId meshInstanceId, glm::mat4 newModel);

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
    MeshId lastMeshId = 1; // Starts from one because of INVALID_MESH_ID
    MeshInstanceId lastMeshInstanceId = 0;

    MeshId getNextMeshId();
    MeshInstanceId getNextMeshInstanceId();

    Ground ground;
    std::vector<Surface> surfaces;

    std::vector<Vehicle> vehicles;
    std::vector<Prop> props;
    std::vector<Trigger> triggers;

    // Environment
    Environment environment;

    MeshId loadOBJ(const std::string &filePath);
    MeshId loadFBX(const std::string &filePath);
    MeshId loadGLB(const std::string &filePath);
    MeshId loadGLTF(const std::string &filePath);

    void makeExampleGround();

    MeshInstanceId addMeshInstance(MeshId meshId);
};