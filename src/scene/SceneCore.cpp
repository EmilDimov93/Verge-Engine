// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include "../shared/HandleFactory.hpp"

#include "../shared/MeshLoader.hpp"
#include "../shared/Log.hpp"

#include <vector>

namespace VE
{

    Scene::Scene()
    {
        // Fallback surface
        surfaceTypes.push_back({1.0f, {0.1f, 0.1f, 0.1f}});
    }

    Player &Scene::player(PlayerHandle handle)
    {
        for (std::unique_ptr<Controller> &controller : controllers)
        {
            if (Player *player = dynamic_cast<Player *>(controller.get()))
            {
                if (handle == player->getHandle())
                {
                    return *player;
                }
            }
        }

        Log::add('S', 202);
        std::unreachable();
    }

    Vehicle &Scene::vehicle(VehicleHandle handle)
    {
        for (Vehicle &vehicle : vehicles)
        {
            if (handle == vehicle.getHandle())
            {
                return vehicle;
            }
        }

        Log::add('S', 203);
        std::unreachable();
    }

    Prop &Scene::prop(PropHandle handle)
    {
        for (Prop &prop : props)
        {
            if (handle == prop.getHandle())
            {
                return prop;
            }
        }

        Log::add('S', 204);
        std::unreachable();
    }

    Trigger &Scene::trigger(TriggerHandle handle)
    {
        for (Trigger &trigger : triggers)
        {
            if (handle == trigger.getHandle())
            {
                return trigger;
            }
        }

        Log::add('S', 205);
        std::unreachable();
    }

    void Scene::tick(milliseconds_t dt, std::vector<std::pair<PlayerHandle, VehicleInputState>> inputData)
    {
        this->dt = dt;

        static constexpr double maxDeltaTime = 0.25;
        if (dt > maxDeltaTime)
            return;

        vehicleRemovedThisFrame = false;
        modelRemovedThisFrame = false;

        for (const auto &vis : inputData)
        {
            player(vis.first).setVehicleInputState(vis.second);
        }

        for (Vehicle &vehicle : vehicles)
        {
            // Get Controller input
            VehicleInputState vis{};
            for (const std::unique_ptr<Controller> &controller : controllers)
            {
                if (controller->getVehicleHandle() == vehicle.getHandle())
                {
                    vis = controller->getVehicleInputState();
                    break;
                }
            }

            // Recalculate velocity vector
            vehicle.tick(vis, environment, sampleSurfaceTypeAt(vehicle.getTransform().position).friction, dt);

            // Update transform with new velocity vector
            vehicle.updateTransform();

            // Collisions
            float totalMaxClimb = vehicle.getTransform().position.y + vehicle.getMaxClimb();

            std::array<float, Vehicle::CollisionPointCount> surfaceHeightAtCollisionPoints;
            float heightAvg = 0.0f;
            for (size_t i = 0; i < Vehicle::CollisionPointCount; i++)
            {
                surfaceHeightAtCollisionPoints[i] = sampleHeightAt(vehicle.getCollisionPointWorld(i));
                heightAvg += surfaceHeightAtCollisionPoints[i];

                if (totalMaxClimb < surfaceHeightAtCollisionPoints[i])
                {
                    vehicle.collideVelocityVector(vehicle.getCollisionPointLocal(i));
                }
            }

            heightAvg /= Vehicle::CollisionPointCount;

            if (vehicle.getTransform().position.y < heightAvg)
            {
                vehicle.setHeight(heightAvg);
                glm::vec3 v = vehicle.getVelocityVector();
                if (v.y < 0.0f)
                    v.y = 0.0f;
                vehicle.setVelocityVector(v);
            }

            setModelMat(vehicle.getBodyModelInstanceHandle(), vehicle.getBodyMat());

            for (size_t i = 0; i < WHEEL_COUNT; i++)
                setModelMat(vehicle.getWheelModelInstanceHandle(static_cast<Wheel>(i)), vehicle.getWheelMat(static_cast<Wheel>(i)));
        }

        {
            oneShotAudioRequests.clear();

            for (EngineAudioRequest &req : engineAudioRequests)
            {
                Vehicle v = vehicle(req.vehicleHandle);
                req.pitch = v.getRpm() / v.getMaxRpm();
                req.position = v.getTransform().position;
            }

            for (LayeredEngineAudioRequest &req : layeredEngineAudioRequests)
            {
                Vehicle v = vehicle(req.vehicleHandle);
                req.rpm = v.getRpm();
                req.maxRpm = v.getMaxRpm();
                req.position = v.getTransform().position;
            }
        }

        for (std::unique_ptr<Controller> &controller : controllers)
        {
            if (Player *player = dynamic_cast<Player *>(controller.get()))
            {
                for (Vehicle &vehicle : vehicles)
                {
                    if (player->getVehicleHandle() == vehicle.getHandle())
                    {
                        player->updateCamera(dt, vehicle.getTransform(), vehicle.getVelocityVector());
                        break;
                    }
                }
            }
        }

        for (Prop &prop : props)
        {
            if (prop.hasChanges())
            {
                setModelMat(prop.getModelInstanceHandle(), prop.getModelMat());
                prop.markChangesSaved();
            }
        }

        {
            for (Trigger &trigger : triggers)
            {
                for (Vehicle &vehicle : vehicles)
                {
                    if (trigger.doesActorTrigger(vehicle.getTransform().position))
                    {
                        trigger.callback();
                        if (trigger.isAutoDestroy())
                        {
                            trigger.markForDestroy();
                            break;
                        }
                    }
                }
            }

            std::vector<TriggerHandle> markedHandles;
            for (const auto &trigger : triggers)
            {
                if (trigger.isMarkedForDestroy())
                    markedHandles.push_back(trigger.getHandle());
            }

            for (const auto &handle : markedHandles)
                removeTrigger(handle);
        }
    }

    ModelHandle Scene::addModel(const std::string &filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();

        if (ext != ".obj")
        {
            Log::add('E', 101);
            return INVALID_MODEL_HANDLE;
        }

        ModelHandle newModelHandle = HandleFactory<ModelHandle>::getNewHandle();

        std::vector<Mesh> meshes = loadOBJ(filePath);

        if (meshes.empty())
        {
            Log::add('E', 102);
            return INVALID_MODEL_HANDLE;
        }

        models.emplace_back(newModelHandle, meshes);

        return newModelHandle;
    }

    float Scene::sampleHeightAt(const Position3 &point) const
    {
        float highest = 0;
        if (surfaces.size() == 0)
        {
            return FLOAT_MIN;
        }
        else
        {
            highest = surfaces[0].sampleHeight(point);
        }

        for (const Surface &surface : surfaces)
        {
            if (surface.position.y + surface.sampleHeight(point) < point.y)
            {
                if (surface.position.y + surface.sampleHeight(point) > highest)
                {
                    highest = surface.position.y + surface.sampleHeight(point);
                }
            }
        }

        return highest;
    }

    const SurfaceType &Scene::sampleSurfaceTypeAt(const Position3 &point) const
    {
        float highest;
        uint32_t highestIndex = 0;
        if (surfaces.size() == 0)
        {
            return surfaceTypes[0]; // Default surface type
        }
        else
        {
            highest = surfaces[0].sampleHeight(point);
        }

        uint32_t index = 0;
        for (const Surface &surface : surfaces)
        {
            if (surface.position.y + surface.sampleHeight(point) < point.y)
            {
                if (surface.position.y + surface.sampleHeight(point) > highest)
                {
                    highest = surface.position.y + surface.sampleHeight(point);
                    highestIndex = index;
                }
            }
            index++;
        }

        return surfaceTypes[surfaces[highestIndex].sampleSurfaceTypeIndex(point)];
    }

    void Scene::setAirDensity(float airDensityKgpm3)
    {
        environment.airDensityKgpm3 = airDensityKgpm3;
    }

    void Scene::setGravity(float gravityMps2)
    {
        environment.gravityMps2 = gravityMps2;
    }

    void Scene::setBackgroundColor(color_t backgroundColor)
    {
        environment.backgroundColor = backgroundColor;
    }

    void Scene::playAudio(std::string fileName, float pitch)
    {
        oneShotAudioRequests.emplace_back(AudioRequest{fileName, pitch, false, {}});
    }

    void Scene::playAudio3D(std::string fileName, float pitch, Position3 position)
    {
        oneShotAudioRequests.emplace_back(AudioRequest{fileName, pitch, true, position});
    }

}
