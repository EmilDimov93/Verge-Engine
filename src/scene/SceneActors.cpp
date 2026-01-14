// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include <glm/gtc/random.hpp>

void Scene::tick(ve_time_t frameTime)
{
    dt = frameTime;

    for (Vehicle &vehicle : vehicles)
    {
        // Temporary(testing)
        vehicle.setHeight(ground.sampleHeight(vehicle.getTransform().position.x, vehicle.getTransform().position.z));

        VehicleInputState vis{};
        for (const std::unique_ptr<Controller> &controller : controllers)
        {
            if (controller->getVehicleId() == vehicle.getId())
            {
                vis = controller->getVehicleInputState();
                break;
            }
        }

        vehicle.tick(vis, environment, surfaces[ground.sampleSurfaceIndex(vehicle.getTransform().position.x, vehicle.getTransform().position.z)].friction, dt);

        setMatrix(vehicle.bodyMeshInstanceId, vehicle.bodyMat);
        setMatrix(vehicle.wheelFLMeshInstanceId, vehicle.wheelFLMat);
        setMatrix(vehicle.wheelFRMeshInstanceId, vehicle.wheelFRMat);
        setMatrix(vehicle.wheelBLMeshInstanceId, vehicle.wheelBLMat);
        setMatrix(vehicle.wheelBRMeshInstanceId, vehicle.wheelBRMat);
    }

    for (std::unique_ptr<Controller> &controller : controllers)
    {
        if (Player *player = dynamic_cast<Player *>(controller.get()))
        {
            for (Vehicle &vehicle : vehicles)
            {
                if (player->getVehicleId() == vehicle.getId())
                {
                    player->updateCamera(dt, vehicle.getTransform(), vehicle.getVelocityVector());
                }
            }
        }
    }

    for (Prop &prop : props)
    {
        setMatrix(prop.meshInstanceId, prop.getModelMat());
    }

    for (Trigger &trigger : triggers)
    {
        for (Vehicle &vehicle : vehicles)
        {
            if (trigger.doesActorTrigger(vehicle.getTransform().position))
            {
                std::cout << "Triggered: " << trigger.getId() << std::endl;
                // call callback function
                if (trigger.getIsAutoDestroy())
                {
                    trigger.markForDestroy();
                    break;
                }
            }
        }
    }
    std::erase_if(triggers, [](const Trigger &t)
                  { return t.getIsMarkedForDestroy(); });
}

DrawData Scene::getDrawData(PlayerId playerId)
{
    for (const std::unique_ptr<Controller> &controller : controllers)
    {
        if (const Player *player = dynamic_cast<const Player *>(controller.get()))
        {
            if (player->getId() == playerId)
            {
                DrawData drawData(meshes, meshInstances, player->camera.getProjectionMatrix(), player->camera.getViewMatrix(), environment.backgroundColor);
                return drawData;
            }
        }
    }

    Log::add('S', 202);
    return DrawData{meshes, meshInstances, {0}, {0}, environment.backgroundColor};
}

void Scene::setMatrix(MeshInstanceId meshInstanceId, glm::mat4 newModel)
{
    for (MeshInstance &instance : meshInstances)
    {
        if (instance.id == meshInstanceId)
        {
            instance.model = newModel;
            break;
        }
    }
}

MeshInstanceId Scene::addMeshInstance(MeshId meshId)
{
    if (meshId == INVALID_MESH_ID)
    {
        Log::add('S', 200);
    }

    bool foundMesh = false;
    for (size_t i = 0; i < meshes.size(); i++)
    {
        if (meshes[i].getId() == meshId)
        {
            foundMesh = true;
            break;
        }
    }

    if (!foundMesh)
    {
        Log::add('S', 200);
    }

    MeshInstance newMeshInstance;
    newMeshInstance.meshId = meshId;
    newMeshInstance.id = getNextMeshInstanceId();

    meshInstances.push_back(newMeshInstance);

    return newMeshInstance.id;
}

VehicleId Scene::addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info, Transform transform)
{
    VehicleId id = getNextVehicleId();

    Vehicle newVehicle(id,
                       transform,
                       info,
                       addMeshInstance(info.bodyMeshId),
                       addMeshInstance(info.wheelMeshId),
                       addMeshInstance(info.wheelMeshId),
                       addMeshInstance(info.wheelMeshId),
                       addMeshInstance(info.wheelMeshId));

    vehicles.push_back(newVehicle);

    return id;
}

void Scene::addProp(MeshId meshId, Transform transform)
{
    MeshInstanceId meshInstanceId = addMeshInstance(meshId);

    Prop newProp(meshInstanceId, transform);
    props.push_back(newProp);

    setMatrix(meshInstanceId, newProp.getModelMat());
}

void Scene::addTrigger(TriggerId id, const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info, Transform transform)
{
    for (Trigger trigger : triggers)
    {
        if (id == trigger.getId())
        {
            Log::add('S', 201);
        }
    }

    MeshInstanceId meshInstanceId = addMeshInstance(info.meshId);

    Trigger newTrigger(id, transform, meshInstanceId, info);
    triggers.push_back(newTrigger);

    setMatrix(meshInstanceId, newTrigger.getModelMat());
}

// Should check unique?
uint64_t Scene::getNextMeshId()
{
    lastMeshId++;
    return lastMeshId;
}

// Should check unique?
MeshInstanceId Scene::getNextMeshInstanceId()
{
    lastMeshInstanceId++;
    return lastMeshInstanceId;
}

VehicleId Scene::getNextVehicleId()
{
    lastVehicleId++;
    return lastVehicleId;
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

    MeshId newMeshId = getNextMeshId();

    Mesh objMesh(newMeshId, meshVertices, meshIndices);

    meshes.push_back(objMesh);

    MeshInstanceId newMeshInstanceId = addMeshInstance(newMeshId);

    setMatrix(newMeshInstanceId, transformToMat(transform));
}