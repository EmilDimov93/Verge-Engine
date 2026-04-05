// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "Player.hpp"

#include "actors/Surface.hpp"
#include "actors/Vehicle.hpp"
#include "actors/Prop.hpp"
#include "actors/Trigger.hpp"

#include "Environment.hpp"

#include "../shared/DrawData.hpp"

class Scene
{
public:
    Scene();

    Player &player(PlayerHandle handle);
    Vehicle &vehicle(VehicleHandle handle);
    Prop &prop(PropHandle handle);
    Trigger &trigger(TriggerHandle handle);

    DrawData getDrawData(PlayerHandle playerHandle);

    AudioData getAudioData(PlayerHandle playerHandle);

    ModelHandle loadFile(const std::string &filePath);

    PlayerHandle addPlayer(VehicleHandle vehicleHandle, const VE_STRUCT_CAMERA_CREATE_INFO &cameraInfo);

    VehicleHandle addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info, Transform transform = {});
    PropHandle addProp(ModelHandle modelHandle, Transform transform);
    TriggerHandle addTrigger(const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info, Transform transform = {});

    void removeVehicle(VehicleHandle handle);
    void removeProp(PropHandle handle);
    void removeTrigger(TriggerHandle handle);

    void tick(ve_time_t dt, std::vector<std::pair<PlayerHandle, VehicleInputState>> inputData);

    SurfaceTypeIndex addSurfaceType(const VE_STRUCT_SURFACE_TYPE_CREATE_INFO &info);
    void addSurface(Size2 size, const std::vector<uint32_t> &surfaceTypeMap, const std::vector<float> &heightMap, float tileSize = 1.0f, Position3 position = {});

    void setAirDensity(float airDensity);
    void setGravity(float gravity);
    void setBackgroundColor(ve_color_t backgroundColor);

    void playAudio(std::string fileName, float pitch);
    void playAudio3D(std::string fileName, float pitch, Position3 position);

private:
    ve_time_t dt;

    // Controllers
    std::vector<std::unique_ptr<Controller>> controllers;

    // Models
    std::vector<Model> models;
    std::vector<ModelInstance> modelInstances;

    // Surface
    std::vector<Surface> surfaces;
    std::vector<SurfaceType> surfaceTypes;

    // Actors
    std::vector<Vehicle> vehicles;
    std::vector<Prop> props;
    std::vector<Trigger> triggers;

    // Environment
    Environment environment;

    // Audio
    std::vector<VEEngineAudioRequest> engineAudioRequests;
    std::vector<VELayeredEngineAudioRequest> layeredEngineAudioRequests;
    std::vector<VEAudioRequest> oneShotAudioRequests;

    ModelHandle loadOBJ(const std::string &filePath);
    ModelHandle loadFBX(const std::string &filePath);
    ModelHandle loadGLB(const std::string &filePath);
    ModelHandle loadGLTF(const std::string &filePath);

    void setModelMat(ModelInstanceHandle modelInstanceHandle, glm::mat4 newModel);

    ModelInstanceHandle addModelInstance(ModelHandle modleHandle);

    bool isModelInstanced(ModelHandle modelHandle) const;

    float sampleHeightAt(const Position3 &point) const;
    const SurfaceType &sampleSurfaceTypeAt(const Position3 &point) const;
    float sampleHeightAt(const glm::vec3 &point) const;
    const SurfaceType &sampleSurfaceTypeAt(const glm::vec3 &point) const;

    bool vehicleRemovedThisFrame = false;
    bool modelRemovedThisFrame = false;
};