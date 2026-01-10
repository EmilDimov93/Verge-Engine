// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <glm/gtc/random.hpp>

#include "../shared/Log.hpp"

Scene::Scene(ve_color_t backgroundColor, const VE_STRUCT_CAMERA_CREATE_INFO &cameraInfo) : camera(cameraInfo)
{
    environment.backgroundColor = backgroundColor;

    isCameraFollowingVehicle = false;

    // Default surface
    surfaces.push_back({1.0f, {0.1f, 0.1f, 0.1f}});
}

DrawData Scene::getDrawData()
{
    DrawData drawData(meshes, meshInstances, camera.getProjectionMatrix(), camera.getViewMatrix(), environment.backgroundColor);
    return drawData;
}

MeshId Scene::loadFile(const std::string &filePath)
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

    return -1;
}

MeshId Scene::loadOBJ(const std::string &filePath)
{
    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndices;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        Log::add('S', 101);
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
                return static_cast<uint32_t>(
                    std::stoi(s.substr(0, s.find('/'))) - 1);
            };

            uint32_t ids[3] = {
                parseIndex(a),
                parseIndex(b),
                parseIndex(c)};

            for (uint32_t idx : ids)
            {
                Vertex v;
                v.pos = positions[idx];
                v.col = currentColor;

                meshIndices.push_back(
                    static_cast<uint32_t>(meshVertices.size()));
                meshVertices.push_back(v);
            }
        }
    }

    MeshId newMeshId = getNextMeshId();

    Mesh objMesh(newMeshId, meshVertices, meshIndices);

    meshes.push_back(objMesh);

    return newMeshId;
}

MeshId Scene::loadFBX(const std::string &filePath)
{
    Log::add('S', 100);
    return -1;
}

MeshId Scene::loadGLB(const std::string &filePath)
{
    Log::add('S', 100);
    return -1;
}

MeshId Scene::loadGLTF(const std::string &filePath)
{
    Log::add('S', 100);
    return -1;
}

MeshInstanceId Scene::addMeshInstance(int64_t meshId)
{
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

void Scene::setAirDensity(float airDensity)
{
    environment.airDensity = airDensity;
}

void Scene::setGravity(float gravity)
{
    environment.gravity = gravity;
}

void Scene::setBackgroundColor(ve_color_t backgroundColor)
{
    environment.backgroundColor = backgroundColor;
}

void Scene::addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info, Transform transform)
{
    Vehicle newVehicle(transform,
                       info,
                       addMeshInstance(info.bodyMeshId),
                       addMeshInstance(info.wheelMeshId),
                       addMeshInstance(info.wheelMeshId),
                       addMeshInstance(info.wheelMeshId),
                       addMeshInstance(info.wheelMeshId));
    vehicles.push_back(newVehicle);
}

void Scene::addProp(MeshId meshId, Transform transform)
{
    uint32_t meshInstanceId = addMeshInstance(meshId);

    Prop newProp(meshInstanceId, transform);
    props.push_back(newProp);

    setMatrix(meshInstanceId, newProp.getModelMat());
}

void Scene::addTrigger(int32_t id, const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info, Transform transform)
{
    for (Trigger trigger : triggers)
    {
        if (id == trigger.getId())
        {
            Log::add('S', 200);
        }
    }

    uint32_t meshInstanceId = addMeshInstance(info.meshId);

    Trigger newTrigger(id, transform, meshInstanceId, info);
    triggers.push_back(newTrigger);

    setMatrix(meshInstanceId, newTrigger.getModelMat());
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

void Scene::tick(ve_time_t frameTime)
{
    dt = frameTime;

    for (Vehicle &vehicle : vehicles)
    {
        // Temporary(testing)
        vehicle.setHeight(ground.sampleHeight(vehicle.getTransform().position.x, vehicle.getTransform().position.z));

        vehicle.tick(environment, surfaces[ground.sampleSurfaceIndex(vehicle.getTransform().position.x, vehicle.getTransform().position.z)].friction, dt);

        setMatrix(vehicle.bodyMeshInstanceId, vehicle.bodyMat);
        setMatrix(vehicle.wheelFLMeshInstanceId, vehicle.wheelFLMat);
        setMatrix(vehicle.wheelFRMeshInstanceId, vehicle.wheelFRMat);
        setMatrix(vehicle.wheelBLMeshInstanceId, vehicle.wheelBLMat);
        setMatrix(vehicle.wheelBRMeshInstanceId, vehicle.wheelBRMat);
    }

    if (isCameraFollowingVehicle)
        cameraFollowVehicle();

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

    camera.tick();
}

void Scene::setCameraFollowVehicle(uint32_t vehicleIndex)
{
    if (vehicleIndex < 0 || vehicleIndex > vehicles.size() + 1)
    {
        Log::add('S', 103);
        return;
    }

    isCameraFollowingVehicle = true;
    cameraFollowedVehicleIndex = vehicleIndex;
}

void Scene::unsetCameraFollowVehicle()
{
    isCameraFollowingVehicle = false;
}

void Scene::setCameraFollowDistance(float distance)
{
    cameraFollowDistance = distance;
}

void Scene::setCameraFollowHeight(float height)
{
    cameraFollowHeight = height;
}

void Scene::setCameraFollowYawDelay(float yawDelay)
{
    cameraFollowYawDelay = yawDelay;
}

void Scene::cameraFollowVehicle()
{
    float currCameraHeight = cameraFollowHeight;
    // Move camera when ground is obstructing view
    /*if(ground.sampleHeight(camera.getPosition().x, camera.getPosition().z) >= currCameraHeight){
        currCameraHeight += ground.sampleHeight(camera.getPosition().x, camera.getPosition().z);
    }*/

    glm::vec3 vehicleVelocityVector = vehicles[cameraFollowedVehicleIndex].getVelocityVector();
    float vehicleYaw = atan2(vehicleVelocityVector.x, vehicleVelocityVector.z);

    static float cameraYaw = vehicleYaw - PI;

    float targetYaw = vehicleYaw - PI;

    cameraYaw += (targetYaw - cameraYaw) * cameraFollowYawDelay;

    Position3 vehiclePos = vehicles[cameraFollowedVehicleIndex].getTransform().position;

    static glm::vec3 prevCamPos = {camera.getPosition().x, camera.getPosition().y, camera.getPosition().z};
    glm::vec3 targetCamPos = {vehiclePos.x + sin(cameraYaw) * cameraFollowDistance, vehiclePos.y + currCameraHeight, vehiclePos.z + cos(cameraYaw) * cameraFollowDistance};
    glm::vec3 newCamPos = glm::mix(prevCamPos, targetCamPos, 1.0f - std::exp(-float(dt) * 10.0f));
    camera.move({newCamPos.x, newCamPos.y, newCamPos.z});
    prevCamPos = {camera.getPosition().x, camera.getPosition().y, camera.getPosition().z};

    glm::vec3 dir = glm::normalize(glm::vec3(vehiclePos.x, vehiclePos.y, vehiclePos.z) - newCamPos);
    float pitch = glm::degrees(asin(dir.y));
    float yaw = glm::degrees(atan2(dir.z, dir.x));
    camera.rotate({pitch, yaw, 0});
}

Scene::~Scene()
{
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
