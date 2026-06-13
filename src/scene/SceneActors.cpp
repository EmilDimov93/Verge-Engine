// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include "../shared/HandleFactory.hpp"

#include <glm/gtc/random.hpp>

namespace VE
{

    SceneDrawData Scene::getDrawData(PlayerHandle playerHandle) const
    {
        for (const std::unique_ptr<Controller> &controller : controllers)
        {
            if (const Player *player = dynamic_cast<const Player *>(controller.get()))
            {
                if (player->getHandle() == playerHandle)
                {
                    SceneDrawData drawData(models, modelInstances, player->getCameraViewMat(), environment.backgroundColor, environment.outdoorBrightness, modelRemovedThisFrame);
                    return drawData;
                }
            }
        }

        Log::add('S', 202);
        std::unreachable();
    }

    AudioData Scene::getAudioData(PlayerHandle playerHandle)
    {
        for (const std::unique_ptr<Controller> &controller : controllers)
        {
            if (const Player *player = dynamic_cast<const Player *>(controller.get()))
            {
                if (player->getHandle() == playerHandle)
                {
                    AudioData audioData(player->getCameraPosition(), player->getCameraYaw(), engineAudioRequests, layeredEngineAudioRequests, oneShotAudioRequests, vehicleRemovedThisFrame);
                    return audioData;
                }
            }
        }

        Log::add('S', 202);
        std::unreachable();
    }

    void Scene::setModelMat(ModelInstanceHandle modelInstanceHandle, glm::mat4 modelMat)
    {
        for (ModelInstance &instance : modelInstances)
        {
            if (instance.handle == modelInstanceHandle)
            {
                instance.modelMat = modelMat;
                break;
            }
        }
    }

    ModelInstanceHandle Scene::addModelInstance(ModelHandle modelHandle)
    {
        if (modelHandle == INVALID_MODEL_HANDLE)
            Log::add('S', 201);

        bool foundModel = false;
        for (const Model &model : models)
        {
            if (model.getHandle() == modelHandle)
            {
                foundModel = true;
                break;
            }
        }

        if (!foundModel)
            Log::add('S', 201);

        modelInstances.emplace_back(HandleFactory<ModelInstanceHandle>::getNewHandle(), modelHandle, glm::mat4(1.0f));

        return modelInstances.back().handle;
    }

    bool Scene::isModelInstanced(ModelHandle modelHandle) const
    {
        for (const ModelInstance &modelInstance : modelInstances)
        {
            if (modelInstance.modelHandle == modelHandle)
            {
                return true;
            }
        }

        return false;
    }

    PlayerHandle Scene::addPlayer(VehicleHandle vehicleHandle)
    {
        PlayerHandle handle = HandleFactory<PlayerHandle>::getNewHandle();

        controllers.push_back(std::make_unique<Player>(handle, vehicleHandle));

        return handle;
    }

    VehicleHandle Scene::addVehicle(const VehicleCreateInfo &info, Transform transform)
    {
        VehicleHandle handle = HandleFactory<VehicleHandle>::getNewHandle();

        Vehicle newVehicle(handle,
                           transform,
                           info,
                           addModelInstance(info.bodyModelHandle),
                           addModelInstance(info.wheelModelHandle),
                           addModelInstance(info.wheelModelHandle),
                           addModelInstance(info.wheelModelHandle),
                           addModelInstance(info.wheelModelHandle));

        if (!info.engineAudioFileName.empty())
        {
            EngineAudioRequest newAudioRequest;
            newAudioRequest.vehicleHandle = handle;
            newAudioRequest.fileName = info.engineAudioFileName;
            newAudioRequest.position = transform.position;

            engineAudioRequests.push_back(newAudioRequest);
        }

        if (!info.layeredEngineAudioFiles.empty())
        {
            LayeredEngineAudioRequest newAudioRequest;
            newAudioRequest.vehicleHandle = handle;
            newAudioRequest.position = transform.position;
            newAudioRequest.audioFiles = info.layeredEngineAudioFiles;

            layeredEngineAudioRequests.push_back(newAudioRequest);
        }

        float maxX = -std::numeric_limits<float>::infinity();
        float minX = std::numeric_limits<float>::infinity();
        float maxY = -std::numeric_limits<float>::infinity();
        float minY = std::numeric_limits<float>::infinity();
        float maxZ = -std::numeric_limits<float>::infinity();
        float minZ = std::numeric_limits<float>::infinity();
        for (const Model &model : models)
        {
            if (info.bodyModelHandle == model.getHandle())
            {
                for (const Mesh &mesh : model.getMeshes())
                {
                    for (const Vertex &v : mesh.getVertices())
                    {
                        if (v.pos.x > maxX)
                            maxX = v.pos.x;

                        if (v.pos.x < minX)
                            minX = v.pos.x;

                        if (v.pos.y > maxY)
                            maxY = v.pos.y;

                        if (v.pos.y < minY)
                            minY = v.pos.y;

                        if (v.pos.z > maxZ)
                            maxZ = v.pos.z;

                        if (v.pos.z < minZ)
                            minZ = v.pos.z;
                    }
                }

                break;
            }
        }

        // Collision points for each corner of the vehicle at the minimum height
        newVehicle.collisionPoints[0] = {maxX, minY, maxZ};
        newVehicle.collisionPoints[1] = {minX, minY, maxZ};
        newVehicle.collisionPoints[2] = {maxX, minY, minZ};
        newVehicle.collisionPoints[3] = {minX, minY, minZ};

        vehicles.push_back(newVehicle);

        return handle;
    }

    PropHandle Scene::addProp(ModelHandle modelHandle, Transform transform, float lightStrength, color_t lightColor)
    {
        PropHandle handle = HandleFactory<PropHandle>::getNewHandle();

        ModelInstanceHandle modelInstanceHandle = addModelInstance(modelHandle);

        modelInstances.back().lightStrength = lightStrength;
        modelInstances.back().lightColor = lightColor;

        Prop newProp(handle, modelInstanceHandle, transform);

        props.push_back(newProp);

        setModelMat(modelInstanceHandle, newProp.getModelMat());

        return handle;
    }

    TriggerHandle Scene::addTrigger(const TriggerTypeCreateInfo &info, Transform transform, const std::function<void()> &callback)
    {
        TriggerHandle handle = HandleFactory<TriggerHandle>::getNewHandle();

        ModelInstanceHandle modelInstanceHandle = addModelInstance(info.modelHandle);

        triggers.emplace_back(handle, transform, modelInstanceHandle, info, callback);

        setModelMat(modelInstanceHandle, triggers.back().getModelMat());

        return handle;
    }

    void Scene::removePlayer(PlayerHandle handle)
    {
        for (auto it = controllers.begin(); it != controllers.end(); ++it)
        {
            Player *player = dynamic_cast<Player *>(it->get());
            if (player && player->getHandle() == handle)
            {
                controllers.erase(it);
                return;
            }
        }
    }

    void Scene::removeVehicle(VehicleHandle handle)
    {
        const ModelInstanceHandle bodyModelInstanceHandle = vehicle(handle).getBodyModelInstanceHandle();
        std::erase_if(modelInstances, [bodyModelInstanceHandle](const auto &modelInstance)
                      { return modelInstance.handle == bodyModelInstanceHandle; });

        for (size_t i = 0; i < WHEEL_COUNT; i++)
        {
            const ModelInstanceHandle wheelModelInstanceHandle = vehicle(handle).getWheelModelInstanceHandle(static_cast<Wheel>(i));
            std::erase_if(modelInstances, [wheelModelInstanceHandle](const auto &modelInstance)
                          { return modelInstance.handle == wheelModelInstanceHandle; });
        }

        size_t modelsRemoved = std::erase_if(models, [this](const auto &model)
                                             { return !isModelInstanced(model.getHandle()); });
        modelRemovedThisFrame = modelsRemoved > 0;

        std::erase_if(vehicles, [handle](const auto &vehicle)
                      { return vehicle.getHandle() == handle; });

        std::erase_if(engineAudioRequests, [handle](const auto &engineAudioRequest)
                      { return engineAudioRequest.vehicleHandle == handle; });

        std::erase_if(layeredEngineAudioRequests, [handle](const auto &layeredEngineAudioRequests)
                      { return layeredEngineAudioRequests.vehicleHandle == handle; });

        vehicleRemovedThisFrame = true;
    }

    void Scene::removeProp(PropHandle handle)
    {
        const ModelInstanceHandle modelInstanceHandle = prop(handle).getModelInstanceHandle();
        std::erase_if(modelInstances, [modelInstanceHandle](const auto &modelInstance)
                      { return modelInstance.handle == modelInstanceHandle; });

        size_t modelsRemoved = std::erase_if(models, [this](const auto &model)
                                             { return !isModelInstanced(model.getHandle()); });
        modelRemovedThisFrame = modelsRemoved > 0;

        std::erase_if(props, [handle](const auto &prop)
                      { return prop.getHandle() == handle; });
    }

    void Scene::removeTrigger(TriggerHandle handle)
    {
        const ModelInstanceHandle modelInstanceHandle = trigger(handle).getModelInstanceHandle();
        std::erase_if(modelInstances, [modelInstanceHandle](const auto &modelInstance)
                      { return modelInstance.handle == modelInstanceHandle; });

        size_t modelsRemoved = std::erase_if(models, [this](const auto &model)
                                             { return !isModelInstanced(model.getHandle()); });
        modelRemovedThisFrame = modelsRemoved > 0;

        std::erase_if(triggers, [handle](const auto &trigger)
                      { return trigger.getHandle() == handle; });
    }

    SurfaceTypeIndex Scene::addSurfaceType(const SurfaceTypeCreateInfo &info)
    {
        SurfaceType newSurfaceType;

        if (info.friction >= 0)
        {
            newSurfaceType.friction = info.friction;
        }
        else
        {
            Log::add('A', 192);
            newSurfaceType.friction = 0;
        }

        if (info.color.r >= 0 && info.color.r <= 1.0f && info.color.g >= 0 && info.color.g <= 1.0f && info.color.b >= 0 && info.color.b <= 1.0f)
        {
            newSurfaceType.color = info.color;
        }
        else
        {
            Log::add('A', 193);
            newSurfaceType.color = {0, 0, 0};
        }

        if (info.colorDistortion.r >= 0 && info.colorDistortion.r <= 1.0f && info.colorDistortion.g >= 0 && info.colorDistortion.g <= 1.0f && info.colorDistortion.b >= 0 && info.colorDistortion.b <= 1.0f)
        {
            newSurfaceType.colorDistortion = info.colorDistortion;
        }
        else
        {
            Log::add('A', 194);
            newSurfaceType.colorDistortion = {0, 0, 0};
        }

        newSurfaceType.heightDistortion = info.heightDistortion;

        surfaceTypes.push_back(newSurfaceType);

        return surfaceTypes.size() - 1;
    }

    void Scene::addSurface(Size2 size, const std::vector<uint32_t> &surfaceTypeMap, const std::vector<float> &heightMap, float tileSize, Position3 position)
    {
        Surface newSurface;

        newSurface.position = position;

        newSurface.resize(size);

        newSurface.surfaceTypeMap = surfaceTypeMap;
        newSurface.heightMap = heightMap;

        newSurface.tileSize = tileSize;

        const float halfW = (newSurface.size.w - 1) * 0.5f;
        const float halfH = (newSurface.size.h - 1) * 0.5f;

        std::vector<glm::vec3> vertexPositions(newSurface.size.h * newSurface.size.w);

        for (size_t i = 0; i < newSurface.size.h; i++)
        {
            for (size_t j = 0; j < newSurface.size.w; j++)
            {
                uint32_t surfaceTypeIndex = newSurface.surfaceTypeMap[i * newSurface.size.w + j];
                if (surfaceTypeIndex >= surfaceTypes.size())
                {
                    Log::add('A', 190);
                    surfaceTypeIndex = 0;
                }

                newSurface.heightMap[i * newSurface.size.w + j] += glm::linearRand(-surfaceTypes[surfaceTypeIndex].heightDistortion, surfaceTypes[surfaceTypeIndex].heightDistortion);

                vertexPositions[i * newSurface.size.w + j] = glm::vec3((float)(j - halfW) * tileSize, newSurface.heightMap[i * newSurface.size.w + j], (float)(i - halfH) * tileSize);
            }
        }

        std::vector<std::vector<Vertex>> verticesByType(surfaceTypes.size());
        std::vector<std::vector<uint32_t>> indicesByType(surfaceTypes.size());
        std::vector<std::unordered_map<uint32_t, uint32_t>> globalToLocalIndex(surfaceTypes.size());

        auto getLocalIndex = [&](uint32_t surfaceTypeIndex, uint32_t globalVertexIndex) -> uint32_t
        {
            std::unordered_map<uint32_t, uint32_t> &remap = globalToLocalIndex[surfaceTypeIndex];
            std::unordered_map<uint32_t, uint32_t>::iterator it = remap.find(globalVertexIndex);
            if (it != remap.end())
                return it->second;
            uint32_t localIndex = (uint32_t)verticesByType[surfaceTypeIndex].size();
            verticesByType[surfaceTypeIndex].emplace_back(vertexPositions[globalVertexIndex]);
            remap.emplace(globalVertexIndex, localIndex);
            return localIndex;
        };

        for (uint32_t z = 0; z < newSurface.size.h - 1; z++)
        {
            for (uint32_t x = 0; x < newSurface.size.w - 1; x++)
            {
                uint32_t v0 = z * newSurface.size.w + x;
                uint32_t v1 = v0 + 1;
                uint32_t v2 = v0 + newSurface.size.w;
                uint32_t v3 = v2 + 1;

                uint32_t cellTypeIndex = newSurface.surfaceTypeMap[v0];
                if (cellTypeIndex >= surfaceTypes.size())
                    cellTypeIndex = 0;

                std::vector<uint32_t> &cellIndices = indicesByType[cellTypeIndex];

                cellIndices.push_back(getLocalIndex(cellTypeIndex, v0));
                cellIndices.push_back(getLocalIndex(cellTypeIndex, v2));
                cellIndices.push_back(getLocalIndex(cellTypeIndex, v1));

                cellIndices.push_back(getLocalIndex(cellTypeIndex, v1));
                cellIndices.push_back(getLocalIndex(cellTypeIndex, v2));
                cellIndices.push_back(getLocalIndex(cellTypeIndex, v3));
            }
        }

        surfaces.push_back(newSurface);

        ModelHandle newModelHandle = HandleFactory<ModelHandle>::getNewHandle();

        std::vector<Material> materials;
        materials.reserve(surfaceTypes.size());
        for (const SurfaceType &surfaceType : surfaceTypes)
            materials.push_back(Material{glm::vec4(surfaceType.color.r, surfaceType.color.g, surfaceType.color.b, 1.f), 0.f, 1.f});

        std::vector<Mesh> meshes;

        for (uint32_t surfaceTypeIndex = 0; surfaceTypeIndex < surfaceTypes.size(); surfaceTypeIndex++)
        {
            if (indicesByType[surfaceTypeIndex].empty())
                continue;
            meshes.emplace_back(verticesByType[surfaceTypeIndex], indicesByType[surfaceTypeIndex], surfaceTypeIndex, Mesh::NO_TEXTURE);
        }

        Model newModel(newModelHandle, meshes, materials);

        models.push_back(newModel);

        ModelInstanceHandle newModelInstanceHandle = addModelInstance(newModelHandle);

        setModelMat(newModelInstanceHandle, Transform(position).toMat());
    }

}