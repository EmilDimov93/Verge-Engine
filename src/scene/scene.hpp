// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "player.hpp"

#include "actors/surface.hpp"
#include "actors/vehicle.hpp"
#include "actors/prop.hpp"
#include "actors/trigger.hpp"

#include "environment.hpp"

#include "../shared/drawData.hpp"

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

        PlayerHandle addPlayer(VehicleHandle vehicleHandle = VehicleHandle::INVALID);
        void removePlayer(PlayerHandle handle);

        VehicleHandle addVehicle(const VehicleCreateInfo &info, Transform transform = {});
        PropHandle addProp(ModelHandle modelHandle, Transform transform, float lightIntensity = 0.f, color_t lightColor = color_t(1.f));
        TriggerHandle addTrigger(const TriggerTypeCreateInfo &info, Transform transform = {}, const std::function<void()>& callback = nullptr);

        void removeVehicle(VehicleHandle handle);
        void removeProp(PropHandle handle);
        void removeTrigger(TriggerHandle handle);

        void tick(seconds_t dt, std::vector<std::pair<PlayerHandle, VehicleInputState>> inputData);

        [[nodiscard]] SurfaceTypeIndex addSurfaceType(const SurfaceTypeCreateInfo &info);
        void addSurface(glm::uvec2 size, const std::vector<uint32_t> &surfaceTypeMap, const std::vector<float> &heightMap, float tileSize = 1.f, glm::vec3 position = {});

        void setAirDensity(float airDensity);
        void setGravity(float gravity);
        void setBackgroundColor(color_t backgroundColor);
        void setOutdoorBrightness(float outdoorBrightness);

        void playAudio(std::string fileName, float pitch);
        void playAudio3D(std::string fileName, float pitch, glm::vec3 position);

    private:
        seconds_t dt;

        // Controllers
        std::vector<std::unique_ptr<Controller>> controllers;

        // Models
        std::vector<Model> models;
        std::vector<ModelInstance> modelInstances;

        // Actors
        std::vector<Vehicle> vehicles;
        std::vector<Prop> props;
        std::vector<Trigger> triggers;

        // Surface
        std::vector<Surface> surfaces;
        std::vector<SurfaceType> surfaceTypes;

        // Environment
        Environment environment;

        // Audio
        std::vector<EngineAudioRequest> engineAudioRequests;
        std::vector<LayeredEngineAudioRequest> layeredEngineAudioRequests;
        std::vector<AudioRequest> oneShotAudioRequests;

        void setModelMat(ModelInstanceHandle modelInstanceHandle, glm::mat4 newModel);

        [[nodiscard]] ModelInstanceHandle addModelInstance(ModelHandle modleHandle);

        [[nodiscard]] bool isModelInstanced(ModelHandle modelHandle) const;

        [[nodiscard]] float sampleHeightAt(const glm::vec3 &point) const;
        [[nodiscard]] const SurfaceType &sampleSurfaceTypeAt(const glm::vec3 &point) const;

        bool vehicleRemovedThisFrame = false;
        bool modelRemovedThisFrame = false;
    };

}