// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include "HandleFactory.hpp"

#include "../shared/Log.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <filesystem>

Scene::Scene(ve_color_t backgroundColor)
{
    environment.backgroundColor = backgroundColor;

    // Default surface
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
    std::terminate();
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
    std::terminate();
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
    std::terminate();
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
    std::terminate();
}

void Scene::tick(ve_time_t frameTime)
{
    dt = frameTime;

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
        vehicle.tick(vis, environment, sampleSurfaceTypeAt({vehicle.getTransform().position.x, vehicle.getTransform().position.y, vehicle.getTransform().position.z}).friction, dt);

        // Update transform with new velocity vector
        vehicle.updateTransform();

        // Collisions
        float totalMaxClimb = vehicle.getTransform().position.y + vehicle.getMaxClimb();

        float surfaceHeightAtFLPOI = sampleHeightAt({vehicle.getFLPOIWorld().x, vehicle.getFLPOIWorld().y, vehicle.getFLPOIWorld().z});
        float surfaceHeightAtFRPOI = sampleHeightAt({vehicle.getFRPOIWorld().x, vehicle.getFRPOIWorld().y, vehicle.getFRPOIWorld().z});
        float surfaceHeightAtBLPOI = sampleHeightAt({vehicle.getBLPOIWorld().x, vehicle.getBLPOIWorld().y, vehicle.getBLPOIWorld().z});
        float surfaceHeightAtBRPOI = sampleHeightAt({vehicle.getBRPOIWorld().x, vehicle.getBRPOIWorld().y, vehicle.getBRPOIWorld().z});

        if (totalMaxClimb < surfaceHeightAtFLPOI)
        {
            vehicle.collideVelocityVector(vehicle.getFLPOILocal());
        }
        else if (totalMaxClimb < surfaceHeightAtFRPOI)
        {
            vehicle.collideVelocityVector(vehicle.getFRPOILocal());
        }
        else if (totalMaxClimb < surfaceHeightAtBLPOI)
        {
            vehicle.collideVelocityVector(vehicle.getBLPOILocal());
        }
        else if (totalMaxClimb < surfaceHeightAtBRPOI)
        {
            vehicle.collideVelocityVector(vehicle.getBRPOILocal());
        }

        float heightAvg = (surfaceHeightAtFLPOI + surfaceHeightAtFRPOI + surfaceHeightAtBLPOI + surfaceHeightAtBRPOI) / 4;
        
        if(vehicle.getTransform().position.y < heightAvg){
            vehicle.setHeight(heightAvg);
            glm::vec3 v = vehicle.getVelocityVector();
            if (v.y < 0.0f)
                v.y = 0.0f;
            vehicle.setVelocityVector(v);
        }

        setModelMat(vehicle.getBodyMeshInstanceHandle(), vehicle.getBodyMat());

        setModelMat(vehicle.getWheelFLMeshInstanceHandle(), vehicle.getWheelFLMat());
        setModelMat(vehicle.getWheelFRMeshInstanceHandle(), vehicle.getWheelFRMat());
        setModelMat(vehicle.getWheelBLMeshInstanceHandle(), vehicle.getWheelBLMat());
        setModelMat(vehicle.getWheelBRMeshInstanceHandle(), vehicle.getWheelBRMat());
    }

    for (VEAudioRequest &req : audioRequests)
    {
        Vehicle v = vehicle(req.vehicleHandle);
        req.pitch = v.getRpm() / v.getMaxRpm();
        req.position = v.getTransform().position;
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
                }
            }
        }
    }

    for (Prop &prop : props)
    {
        if (prop.hasChanges())
        {
            setModelMat(prop.getMeshInstanceHandle(), prop.getModelMat());
            prop.markChangesSaved();
        }
    }

    for (Trigger &trigger : triggers)
    {
        for (Vehicle &vehicle : vehicles)
        {
            if (trigger.doesActorTrigger(vehicle.getTransform().position))
            {
                std::cout << "Triggered: " << trigger.getHandle().getValue() << std::endl;
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

MeshHandle Scene::loadFile(const std::string &filePath)
{
    std::string ext = std::filesystem::path(filePath).extension().string();

    if (ext == ".obj")
    {
        return loadOBJ(filePath);
    }
    else if (ext == ".fbx")
    {
        return loadFBX(filePath);
    }
    else if (ext == ".glb")
    {
        return loadGLB(filePath);
    }
    else if (ext == ".gltf")
    {
        return loadGLTF(filePath);
    }
    else
    {
        Log::add('S', 100);
    }

    return INVALID_MESH_HANDLE;
}

MeshHandle Scene::loadOBJ(const std::string &filePath)
{
    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndices;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        Log::add('S', 101);
        return INVALID_MESH_HANDLE;
    }

    std::vector<glm::vec3> positions;
    std::unordered_map<std::string, glm::vec3> materials;

    ve_color_t currentColor(1.0f);

    auto trim = [](std::string &s)
    {
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
        s.erase(0, s.find_first_not_of(" \t\r\n"));
    };

    auto loadMTL = [&](const std::string &mtlPath)
    {
        std::ifstream mtl(mtlPath);
        if (!mtl.is_open())
            return;

        std::string line;
        std::string currentMat;

        while (std::getline(mtl, line))
        {
            if (line.rfind("newmtl ", 0) == 0)
            {
                currentMat = line.substr(7);
                trim(currentMat);
            }
            else if (line.rfind("Kd ", 0) == 0 && !currentMat.empty())
            {
                std::stringstream ss(line.substr(3));
                glm::vec3 kd;
                ss >> kd.r >> kd.g >> kd.b;
                materials[currentMat] = kd;
            }
        }
    };

    std::filesystem::path objPath(filePath);

    std::string line;
    while (std::getline(file, line))
    {
        if (line.rfind("mtllib ", 0) == 0)
        {
            std::string mtlFile = line.substr(7);
            trim(mtlFile);
            loadMTL((objPath.parent_path() / mtlFile).string());
        }
        else if (line.rfind("usemtl ", 0) == 0)
        {
            std::string mat = line.substr(7);
            trim(mat);

            auto it = materials.find(mat);
            if (it != materials.end())
                currentColor = it->second;
        }
        else if (line.rfind("v ", 0) == 0)
        {
            glm::vec3 p;
            std::stringstream ss(line.substr(2));
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (line.rfind("f ", 0) == 0)
        {
            std::stringstream ss(line.substr(2));
            std::string a, b, c;
            ss >> a >> b >> c;

            auto parseIndex = [](const std::string &s)
            {
                return static_cast<uint32_t>(std::stoi(s.substr(0, s.find('/'))) - 1);
            };

            uint32_t ids[3] = {parseIndex(a), parseIndex(b), parseIndex(c)};

            for (uint32_t idx : ids)
            {
                Vertex v;
                v.pos = positions[idx];
                v.col = currentColor;

                meshIndices.push_back(static_cast<uint32_t>(meshVertices.size()));
                meshVertices.push_back(v);
            }
        }
    }

    MeshHandle newMeshHandle = HandleFactory<MeshHandle>::getNewHandle();

    Mesh newMesh(newMeshHandle, meshVertices, meshIndices);

    meshes.push_back(newMesh);

    return newMeshHandle;
}

MeshHandle Scene::loadFBX(const std::string &filePath)
{
    Log::add('S', 100);
    return INVALID_MESH_HANDLE;
}

MeshHandle Scene::loadGLB(const std::string &filePath)
{
    Log::add('S', 100);
    return INVALID_MESH_HANDLE;
}

MeshHandle Scene::loadGLTF(const std::string &filePath)
{
    Log::add('S', 100);
    return INVALID_MESH_HANDLE;
}

float Scene::sampleHeightAt(const Position3 &point) const
{
    float highest = 0;
    if (surfaces.size() == 0)
    {
        return FLOAT_MIN; // No surface
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

void Scene::setBackgroundColor(ve_color_t backgroundColor)
{
    environment.backgroundColor = backgroundColor;
}
