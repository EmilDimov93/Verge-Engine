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

        // Audio
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

        for (Trigger &trigger : triggers)
        {
            for (Vehicle &vehicle : vehicles)
            {
                if (trigger.doesActorTrigger(vehicle.getTransform().position))
                {
                    std::cout << "Triggered: " << trigger.getHandle().getValue() << std::endl;
                    // call callback function
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

    ModelHandle Scene::loadFile(const std::string &filePath)
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

        return INVALID_MODEL_HANDLE;
    }

    ModelHandle Scene::loadOBJ(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            Log::add('S', 101);
            return INVALID_MODEL_HANDLE;
        }

        struct Material
        {
            color_t diffuseColor{1.0f};
            std::string diffuseTexturePath;
        };

        std::vector<glm::vec3> positions;
        std::unordered_map<std::string, Material> materials;
        std::vector<glm::vec2> texCoords;

        color_t currentColor(1.0f);
        std::string currentTexturePath;

        std::vector<Vertex> currentMeshVertices;
        std::vector<uint32_t> currentMeshIndices;

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
                if (line.starts_with("newmtl "))
                {
                    currentMat = line.substr(7);
                    trim(currentMat);
                }
                else if (line.starts_with("Kd ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(3));
                    glm::vec3 kd;
                    ss >> kd.r >> kd.g >> kd.b;
                    materials[currentMat].diffuseColor = kd;
                }
                else if (line.starts_with("map_Kd ") && !currentMat.empty())
                {
                    std::string texturePath = line.substr(7);
                    trim(texturePath);
                    std::filesystem::path resolvedPath = std::filesystem::path(mtlPath).parent_path() / texturePath;
                    materials[currentMat].diffuseTexturePath = resolvedPath.string();
                }
            }
        };

        std::vector<Mesh> meshes;
        auto finalizeCurrentMesh = [&]()
        {
            if (currentMeshVertices.empty())
                return;

            Mesh newMesh(currentMeshVertices, currentMeshIndices, currentTexturePath);
            meshes.push_back(newMesh);

            currentMeshVertices.clear();
            currentMeshIndices.clear();
            currentTexturePath.clear();
        };

        std::filesystem::path objPath(filePath);

        std::string line;
        while (std::getline(file, line))
        {
            if (line.starts_with("mtllib "))
            {
                std::string mtlFile = line.substr(7);
                trim(mtlFile);
                loadMTL((objPath.parent_path() / mtlFile).string());
            }
            else if (line.starts_with("usemtl "))
            {
                finalizeCurrentMesh();

                std::string mat = line.substr(7);
                trim(mat);

                auto it = materials.find(mat);
                if (it != materials.end())
                    currentColor = it->second.diffuseColor;
                currentTexturePath = it->second.diffuseTexturePath;
            }
            else if (line.starts_with("v "))
            {
                glm::vec3 p;
                std::stringstream ss(line.substr(2));
                ss >> p.x >> p.y >> p.z;
                positions.push_back(p);
            }
            else if (line.starts_with("f "))
            {
                std::stringstream ss(line.substr(2));
                std::string a, b, c;
                ss >> a >> b >> c;

                auto parseIndices = [](const std::string &token) -> std::pair<uint32_t, int32_t>
                {
                    size_t firstSlash = token.find('/');
                    uint32_t positionIndex = static_cast<uint32_t>(std::stoi(token.substr(0, firstSlash)) - 1);
                    int32_t texCoordIndex = -1;

                    if (firstSlash != std::string::npos && firstSlash + 1 < token.size() && token[firstSlash + 1] != '/')
                    {
                        size_t secondSlash = token.find('/', firstSlash + 1);
                        texCoordIndex = std::stoi(token.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;
                    }

                    return {positionIndex, texCoordIndex};
                };

                std::pair<uint32_t, int32_t> parsed[3] = {parseIndices(a), parseIndices(b), parseIndices(c)};

                for (auto &[positionIndex, texCoordIndex] : parsed)
                {
                    Vertex vertex;
                    vertex.pos = positions[positionIndex];
                    vertex.col = currentColor;
                    vertex.tex = (texCoordIndex >= 0) ? texCoords[texCoordIndex] : glm::vec2(0.0f);
                    vertex.tex.y = 1.0f - vertex.tex.y;

                    currentMeshIndices.push_back(static_cast<uint32_t>(currentMeshVertices.size()));
                    currentMeshVertices.push_back(vertex);
                }
            }
            else if (line.starts_with("vt "))
            {
                glm::vec2 uv;
                std::stringstream ss(line.substr(3));
                ss >> uv.x >> uv.y;
                texCoords.push_back(uv);
            }
            else if (line.starts_with("o "))
            {
                finalizeCurrentMesh();
            }
        }

        finalizeCurrentMesh();

        ModelHandle newModelHandle = HandleFactory<ModelHandle>::getNewHandle();

        models.emplace_back(newModelHandle, meshes);

        return newModelHandle;
    }

    ModelHandle Scene::loadFBX(const std::string &filePath)
    {
        Log::add('S', 100);
        return INVALID_MODEL_HANDLE;
    }

    ModelHandle Scene::loadGLB(const std::string &filePath)
    {
        Log::add('S', 100);
        return INVALID_MODEL_HANDLE;
    }

    ModelHandle Scene::loadGLTF(const std::string &filePath)
    {
        Log::add('S', 100);
        return INVALID_MODEL_HANDLE;
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

    float Scene::sampleHeightAt(const glm::vec3 &pos) const { return sampleHeightAt(Position3(pos.x, pos.y, pos.z)); }

    const SurfaceType &Scene::sampleSurfaceTypeAt(const glm::vec3 &pos) const { return sampleSurfaceTypeAt(Position3(pos.x, pos.y, pos.z)); }

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
