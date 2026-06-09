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

namespace VE
{

    class Scene
    {
    public:
        Scene();

        Player &player(PlayerHandle handle);
        Vehicle &vehicle(VehicleHandle handle);
        Prop &prop(PropHandle handle);
        Trigger &trigger(TriggerHandle handle);

        [[nodiscard]] SceneDrawData getDrawData(PlayerHandle playerHandle) const;

        [[nodiscard]] AudioData getAudioData(PlayerHandle playerHandle);

        [[nodiscard]] ModelHandle addModel(const std::string &filePath);

        PlayerHandle addPlayer(VehicleHandle vehicleHandle);
        void removePlayer(PlayerHandle handle);

        VehicleHandle addVehicle(const VehicleCreateInfo &info, Transform transform = {});
        PropHandle addProp(ModelHandle modelHandle, Transform transform, float lightStrength = 0.0f, color_t lightColor = color_t(1.0f));
        TriggerHandle addTrigger(const TriggerTypeCreateInfo &info, Transform transform = {}, const std::function<void()>& callback = nullptr);

        void removeVehicle(VehicleHandle handle);
        void removeProp(PropHandle handle);
        void removeTrigger(TriggerHandle handle);

        void tick(milliseconds_t dt, std::vector<std::pair<PlayerHandle, VehicleInputState>> inputData);

        [[nodiscard]] SurfaceTypeIndex addSurfaceType(const SurfaceTypeCreateInfo &info);
        void addSurface(Size2 size, const std::vector<uint32_t> &surfaceTypeMap, const std::vector<float> &heightMap, float tileSize = 1.0f, Position3 position = {});

        void setAirDensity(float airDensity);
        void setGravity(float gravity);
        void setBackgroundColor(color_t backgroundColor);
        void setOutdoorBrightness(float outdoorBrightness);

        void playAudio(std::string fileName, float pitch);
        void playAudio3D(std::string fileName, float pitch, Position3 position);

    private:
        milliseconds_t dt;

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
        std::vector<EngineAudioRequest> engineAudioRequests;
        std::vector<LayeredEngineAudioRequest> layeredEngineAudioRequests;
        std::vector<AudioRequest> oneShotAudioRequests;

        void setModelMat(ModelInstanceHandle modelInstanceHandle, glm::mat4 newModel);

        [[nodiscard]] ModelInstanceHandle addModelInstance(ModelHandle modleHandle);

        [[nodiscard]] bool isModelInstanced(ModelHandle modelHandle) const;

        [[nodiscard]] float sampleHeightAt(const Position3 &point) const;
        [[nodiscard]] const SurfaceType &sampleSurfaceTypeAt(const Position3 &point) const;

        bool vehicleRemovedThisFrame = false;
        bool modelRemovedThisFrame = false;
    };

}