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
    return DrawData{meshes, meshInstances, {0}, {0}, environment.backgroundColor};
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
        Log::add('S', 200);
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
        Log::add('S', 200);
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

void Scene::makeExampleGround()
{
    for (uint8_t &tile : ground.surfaceMap)
    {
        tile = 1;
    }

    const int roadHalfWidth = 10;
    const float curveStrength = 40.0f;
    const float curveFrequency = 0.05f;

    for (size_t i = 0; i < ground.h; i++)
    {
        int centerX = static_cast<int>(ground.w / 2 + std::cos(i * curveFrequency) * curveStrength);

        for (int j = centerX - roadHalfWidth; j <= centerX + roadHalfWidth; j++)
        {
            if (j >= 0 && j < (int)ground.w)
            {
                ground.setSurfaceAt(j, i, 2);
                ground.setHeightAt(j, i, 3.0f);
            }
        }
    }
}

uint32_t Scene::addSurface(const VE_STRUCT_SURFACE_CREATE_INFO &info)
{
    Surface newSurface;

    if (info.friction >= 0)
    {
        newSurface.friction = info.friction;
    }
    else
    {
        Log::add('A', 192);
        newSurface.friction = 0;
    }

    if (info.color.r >= 0 && info.color.r <= 1.0f && info.color.g >= 0 && info.color.g <= 1.0f && info.color.b >= 0 && info.color.b <= 1.0f)
    {
        newSurface.color = info.color;
    }
    else
    {
        Log::add('A', 193);
        newSurface.color = {0, 0, 0};
    }

    if (info.colorDistortion.r >= 0 && info.colorDistortion.r <= 1.0f && info.colorDistortion.g >= 0 && info.colorDistortion.g <= 1.0f && info.colorDistortion.b >= 0 && info.colorDistortion.b <= 1.0f)
    {
        newSurface.colorDistortion = info.colorDistortion;
    }
    else
    {
        Log::add('A', 194);
        newSurface.colorDistortion = {0, 0, 0};
    }

    newSurface.heightDistortion = info.heightDistortion;

    surfaces.push_back(newSurface);

    return surfaces.size() - 1;
}

void Scene::buildGroundMesh(Size2 size, Transform transform)
{
    ground.transform = transform;

    ground.resize(size);

    for (uint8_t &tile : ground.surfaceMap)
    {
        tile = 0;
    }

    // Temporary
    makeExampleGround();

    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndices;

    for (size_t i = 0; i < ground.h; i++)
    {
        for (size_t j = 0; j < ground.w; j++)
        {
            uint8_t surfaceIndex = ground.getSurfaceAt(j, i);
            if (surfaceIndex < 0 || surfaceIndex >= surfaces.size())
            {
                Log::add('A', 190);
                surfaceIndex = 0;
            }

            glm::vec3 surfaceColor;
            surfaceColor.r = surfaces[surfaceIndex].color.r + glm::linearRand(-surfaces[surfaceIndex].colorDistortion.r, surfaces[surfaceIndex].colorDistortion.r);
            surfaceColor.g = surfaces[surfaceIndex].color.g + glm::linearRand(-surfaces[surfaceIndex].colorDistortion.g, surfaces[surfaceIndex].colorDistortion.g);
            surfaceColor.b = surfaces[surfaceIndex].color.b + glm::linearRand(-surfaces[surfaceIndex].colorDistortion.b, surfaces[surfaceIndex].colorDistortion.b);

            ground.heightMap[i * ground.w + j] += glm::linearRand(-surfaces[surfaceIndex].heightDistortion, surfaces[surfaceIndex].heightDistortion);

            const float halfW = (ground.w - 1) * 0.5f;
            const float halfH = (ground.h - 1) * 0.5f;
            meshVertices.push_back({{(float)(j - halfW), ground.heightMap[i * ground.w + j], (float)(i - halfH)}, surfaceColor});
        }
    }

    for (uint32_t z = 0; z < ground.h - 1; z++)
    {
        for (uint32_t x = 0; x < ground.w - 1; x++)
        {
            uint32_t v0 = z * ground.w + x;
            uint32_t v1 = v0 + 1;
            uint32_t v2 = v0 + ground.w;
            uint32_t v3 = v2 + 1;

            meshIndices.push_back(v0);
            meshIndices.push_back(v2);
            meshIndices.push_back(v1);

            meshIndices.push_back(v1);
            meshIndices.push_back(v2);
            meshIndices.push_back(v3);
        }
    }

    MeshHandle newMeshHandle = HandleFactory<MeshHandle>::getNewHandle();

    Mesh newMesh(newMeshHandle, meshVertices, meshIndices);

    meshes.push_back(newMesh);

    MeshInstanceHandle newMeshInstanceHandle = addMeshInstance(newMeshHandle);

    setModelMat(newMeshInstanceHandle, transformToMat(transform));
}