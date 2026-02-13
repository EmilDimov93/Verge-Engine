// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include "HandleFactory.hpp"

#include <glm/gtc/random.hpp>

DrawData Scene::getDrawData(PlayerHandle playerHandle)
{
    for (const std::unique_ptr<Controller> &controller : controllers)
    {
        if (const Player *player = dynamic_cast<const Player *>(controller.get()))
        {
            if (player->getHandle() == playerHandle)
            {
                DrawData drawData(meshes, meshInstances, player->getCameraProjectionMat(), player->getCameraViewMat(), environment.backgroundColor);
                return drawData;
            }
        }
    }

    Log::add('S', 202);
    std::terminate();
}

AudioData Scene::getAudioData(PlayerHandle playerHandle)
{
    for (const std::unique_ptr<Controller> &controller : controllers)
    {
        if (const Player *player = dynamic_cast<const Player *>(controller.get()))
        {
            if (player->getHandle() == playerHandle)
            {
                AudioData audioData(player->getCameraPosition(), player->getCameraYaw(), player->getVolume(), engineAudioRequests, oneShotAudioRequests);
                return audioData;
            }
        }
    }

    Log::add('S', 202);
    std::terminate();
}

void Scene::setModelMat(MeshInstanceHandle meshInstanceHandle, glm::mat4 modelMat)
{
    for (MeshInstance &instance : meshInstances)
    {
        if (instance.handle == meshInstanceHandle)
        {
            instance.modelMat = modelMat;
            break;
        }
    }
}

MeshInstanceHandle Scene::addMeshInstance(MeshHandle meshHandle)
{
    if (meshHandle == INVALID_MESH_HANDLE)
    {
        Log::add('S', 201);
    }

    bool foundMesh = false;
    for (size_t i = 0; i < meshes.size(); i++)
    {
        if (meshes[i].getHandle() == meshHandle)
        {
            foundMesh = true;
            break;
        }
    }

    if (!foundMesh)
    {
        Log::add('S', 201);
    }

    MeshInstance newMeshInstance(HandleFactory<MeshInstanceHandle>::getNewHandle(), meshHandle, glm::mat4(1.0f));

    meshInstances.push_back(newMeshInstance);

    return newMeshInstance.handle;
}

PlayerHandle Scene::addPlayer(VehicleHandle vehicleHandle, const PlayerKeybinds &keybinds, const VE_STRUCT_CAMERA_CREATE_INFO &cameraInfo)
{
    PlayerHandle handle = HandleFactory<PlayerHandle>::getNewHandle();

    controllers.push_back(std::make_unique<Player>(handle, vehicleHandle, keybinds, cameraInfo));

    return handle;
}

VehicleHandle Scene::addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info, Transform transform)
{
    VehicleHandle handle = HandleFactory<VehicleHandle>::getNewHandle();

    Vehicle newVehicle(handle,
                       transform,
                       info,
                       addMeshInstance(info.bodyMeshHandle),
                       addMeshInstance(info.wheelMeshHandle),
                       addMeshInstance(info.wheelMeshHandle),
                       addMeshInstance(info.wheelMeshHandle),
                       addMeshInstance(info.wheelMeshHandle));

    if (!info.engineAudioFileName.empty())
    {
        VEAudioRequest newAudioRequest;
        newAudioRequest.vehicleHandle = handle;
        newAudioRequest.fileName = info.engineAudioFileName;
        newAudioRequest.position = transform.position;

        engineAudioRequests.push_back(newAudioRequest);
    }

    float maxX = -std::numeric_limits<float>::infinity();
    float minX = std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    float minY = std::numeric_limits<float>::infinity();
    float maxZ = -std::numeric_limits<float>::infinity();
    float minZ = std::numeric_limits<float>::infinity();
    for (const Mesh &mesh : meshes)
    {
        if (info.bodyMeshHandle == mesh.getHandle())
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

            break;
        }
    }

    float vehicleHeightMin = minY;
    newVehicle.flPOI = {maxX, vehicleHeightMin, maxZ};
    newVehicle.frPOI = {minX, vehicleHeightMin, maxZ};
    newVehicle.blPOI = {maxX, vehicleHeightMin, minZ};
    newVehicle.brPOI = {minX, vehicleHeightMin, minZ};

    vehicles.push_back(newVehicle);

    return handle;
}

PropHandle Scene::addProp(MeshHandle meshHandle, Transform transform)
{
    PropHandle handle = HandleFactory<PropHandle>::getNewHandle();

    MeshInstanceHandle meshInstanceHandle = addMeshInstance(meshHandle);

    Prop newProp(handle, meshInstanceHandle, transform);

    props.push_back(newProp);

    setModelMat(meshInstanceHandle, newProp.getModelMat());

    return handle;
}

TriggerHandle Scene::addTrigger(const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info, Transform transform)
{
    TriggerHandle handle = HandleFactory<TriggerHandle>::getNewHandle();

    MeshInstanceHandle meshInstanceHandle = addMeshInstance(info.meshHandle);

    triggers.emplace_back(handle, transform, meshInstanceHandle, info);

    setModelMat(meshInstanceHandle, triggers.back().getModelMat());

    return handle;
}

uint32_t Scene::addSurfaceType(const VE_STRUCT_SURFACE_TYPE_CREATE_INFO &info)
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

void Scene::addSurface(Size2 size, const std::vector<uint32_t> &surfaceTypeMap, const std::vector<float> &heightMap, Position3 position)
{
    Surface newSurface;

    newSurface.position = position;

    newSurface.resize(size);

    newSurface.surfaceTypeMap = surfaceTypeMap;
    newSurface.heightMap = heightMap;

    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndices;

    for (size_t i = 0; i < newSurface.h; i++)
    {
        for (size_t j = 0; j < newSurface.w; j++)
        {
            uint32_t surfaceTypeIndex = newSurface.surfaceTypeMap[i * newSurface.w + j];
            if (surfaceTypeIndex < 0 || surfaceTypeIndex >= surfaceTypes.size())
            {
                Log::add('A', 190);
                surfaceTypeIndex = 0;
            }

            glm::vec3 surfaceColor;
            surfaceColor.r = surfaceTypes[surfaceTypeIndex].color.r + glm::linearRand(-surfaceTypes[surfaceTypeIndex].colorDistortion.r, surfaceTypes[surfaceTypeIndex].colorDistortion.r);
            surfaceColor.g = surfaceTypes[surfaceTypeIndex].color.g + glm::linearRand(-surfaceTypes[surfaceTypeIndex].colorDistortion.g, surfaceTypes[surfaceTypeIndex].colorDistortion.g);
            surfaceColor.b = surfaceTypes[surfaceTypeIndex].color.b + glm::linearRand(-surfaceTypes[surfaceTypeIndex].colorDistortion.b, surfaceTypes[surfaceTypeIndex].colorDistortion.b);

            newSurface.heightMap[i * newSurface.w + j] += glm::linearRand(-surfaceTypes[surfaceTypeIndex].heightDistortion, surfaceTypes[surfaceTypeIndex].heightDistortion);

            const float halfW = (newSurface.w - 1) * 0.5f;
            const float halfH = (newSurface.h - 1) * 0.5f;
            meshVertices.push_back({{(float)(j - halfW), newSurface.heightMap[i * newSurface.w + j], (float)(i - halfH)}, surfaceColor});
        }
    }

    for (uint32_t z = 0; z < newSurface.h - 1; z++)
    {
        for (uint32_t x = 0; x < newSurface.w - 1; x++)
        {
            uint32_t v0 = z * newSurface.w + x;
            uint32_t v1 = v0 + 1;
            uint32_t v2 = v0 + newSurface.w;
            uint32_t v3 = v2 + 1;

            meshIndices.push_back(v0);
            meshIndices.push_back(v2);
            meshIndices.push_back(v1);

            meshIndices.push_back(v1);
            meshIndices.push_back(v2);
            meshIndices.push_back(v3);
        }
    }

    surfaces.push_back(newSurface);

    MeshHandle newMeshHandle = HandleFactory<MeshHandle>::getNewHandle();

    Mesh newMesh(newMeshHandle, meshVertices, meshIndices);

    meshes.push_back(newMesh);

    MeshInstanceHandle newMeshInstanceHandle = addMeshInstance(newMeshHandle);

    setModelMat(newMeshInstanceHandle, transformToMat({position}));
}