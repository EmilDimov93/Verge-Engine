// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "../Log.hpp"

Scene::Scene(VulkanContext newVulkanContext, float newFov, float newAspectRatio, float newZNear, float newZFar)
{
    vulkanContext = newVulkanContext;

    isCameraFollowingVehicle = false;

    Camera::init(newFov, newAspectRatio, newZNear, newZFar);
}

uint32_t Scene::loadFile(const std::string &filePath){
    std::string ext = std::filesystem::path(filePath).extension().string();

    if(ext == ".obj"){
        return loadOBJ(filePath);
    }
    else if(ext == ".fbx"){
        return loadFBX(filePath);
    }
    else if(ext == ".glb"){
        return loadGLB(filePath);
    }
    else if(ext == ".gltf"){
        return loadGLTF(filePath);
    }
    else{
        Log::add('S', 100);
    }

    return -1;
}

uint32_t Scene::loadOBJ(const std::string &filePath)
{
    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndeces;

    std::ifstream file(filePath);
    if (!file.is_open())
        throw std::runtime_error("Cannot open OBJ file.");

    std::vector<glm::vec3> positions;
    std::unordered_map<std::string, glm::vec3> materials;

    glm::vec3 currentColor(1.0f);

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

                meshIndeces.push_back(
                    static_cast<uint32_t>(meshVertices.size()));
                meshVertices.push_back(v);
            }
        }
    }

    Mesh objMesh;
    // should vkcheck
    objMesh.init(vulkanContext, &meshVertices, &meshIndeces);
    meshes.push_back(objMesh);

    return meshes.size() - 1;
}

uint32_t Scene::loadFBX(const std::string &filePath)
{
    Log::add('S', 100);
    return -1;
}

uint32_t Scene::loadGLB(const std::string &filePath)
{
    Log::add('S', 100);
    return -1;
}

uint32_t Scene::loadGLTF(const std::string &filePath)
{
    Log::add('S', 100);
    return -1;
}

uint32_t Scene::addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info)
{
    Vehicle newVehicle(info);
    vehicles.push_back(newVehicle);

    return vehicles.size() - 1;
}

uint32_t Scene::addProp(uint32_t meshIndex, Position3 position, Rotation3 rotation)
{
    Prop newProp(meshIndex, position, rotation);
    props.push_back(newProp);

    setMatrix(meshIndex, newProp.getModelMat());

    return props.size() - 1;
}

uint32_t Scene::addTrigger(uint32_t id, Position3 position, const VE_STRUCT_TRIGGER_TYPE_CREATE_INFO &info)
{
    for (Trigger trigger : triggers)
    {
        if (id == trigger.getId())
        {
            // Error: id exists
        }
    }
    Trigger newTrigger(id, position, info);
    triggers.push_back(newTrigger);

    return triggers.size() - 1;
}

void Scene::setMatrix(int modelId, glm::mat4 newModel)
{
    if (modelId >= meshes.size() || modelId < 0)
    {
        Log::add('S', 101);
        return;
    }

    meshes[modelId].setModel(newModel);
}

void Scene::tick(ve_time dt)
{
    for (Vehicle &vehicle : vehicles)
    {
        setMatrix(vehicle.bodyMeshIndex, vehicle.bodyMat);
        setMatrix(vehicle.wheelFLMeshIndex, vehicle.wheelFLMat);
        setMatrix(vehicle.wheelFRMeshIndex, vehicle.wheelFRMat);
        setMatrix(vehicle.wheelBLMeshIndex, vehicle.wheelBLMat);
        setMatrix(vehicle.wheelBRMeshIndex, vehicle.wheelBRMat);

        vehicle.tick(dt);
    }

    if (isCameraFollowingVehicle)
        cameraFollowVehicle(dt);

    for (Trigger &trigger : triggers)
    {
        for (Vehicle &vehicle : vehicles)
        {
            if (trigger.doesActorTrigger(vehicle.getPosition()))
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

    Camera::tick();
}

void Scene::setCameraFollowVehicle(uint32_t vehicleIndex)
{
    isCameraFollowingVehicle = true;
    cameraFollowedVehicleIndex = vehicleIndex;
}

void Scene::unsetCameraFollowVehicle()
{
    isCameraFollowingVehicle = false;
}

void Scene::cameraFollowVehicle(ve_time dt)
{
    /*static float cameraRot = -180.0f;
    if (Input::isDown(VE_KEY_LEFT))
    {
        cameraRot += dt * 90;
    }
    if (Input::isDown(VE_KEY_RIGHT))
    {
        cameraRot -= dt * 90;
    }*/

    float cameraRot = vehicles[cameraFollowedVehicleIndex].getMoveDirection().yaw - PI;

    float distance = 8.0f;
    float height = 3.0f;

    Position3 vehiclePos = vehicles[cameraFollowedVehicleIndex].getPosition();

    static glm::vec3 prevCamPos = {Camera::getPosition().x, Camera::getPosition().y, Camera::getPosition().z};
    glm::vec3 targetCamPos = {vehiclePos.x + sin(cameraRot) * distance, vehiclePos.y + height, vehiclePos.z + cos(cameraRot) * distance};
    glm::vec3 newCamPos = glm::mix(prevCamPos, targetCamPos, std::exp(-dt * 10.0f));
    Camera::move({newCamPos.x, newCamPos.y, newCamPos.z});
    prevCamPos = {Camera::getPosition().x, Camera::getPosition().y, Camera::getPosition().z};

    glm::vec3 dir = glm::normalize(glm::vec3(vehiclePos.x, vehiclePos.y, vehiclePos.z) - targetCamPos);
    float pitch = glm::degrees(asin(dir.y));
    float yaw = glm::degrees(atan2(dir.z, dir.x));
    Camera::rotate({pitch, yaw, 0});
}

Scene::~Scene()
{
    for (auto &mesh : meshes)
        mesh.destroyBuffers(vulkanContext.device);
}
