// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include <sstream>
#include <vector>

Scene::Scene(VulkanContext newVulkanContext, float newFov, float newAspectRatio, float newZNear, float newZFar)
{
    vulkanContext = newVulkanContext;

    isCameraFollowingVehicle = false;

    Camera::init(newFov, newAspectRatio, newZNear, newZFar);
}

uint32_t Scene::addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info)
{
    Vehicle newVehicle;
    newVehicle.init(info);
    vehicles.push_back(newVehicle);

    return vehicles.size() - 1;
}

void Scene::loadFile(std::string filename, glm::vec3 color)
{
    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndeces;

    {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Cannot open OBJ file.");

        std::vector<glm::vec3> positions;
        std::string line;

        while (std::getline(file, line))
        {
            if (line.rfind("v ", 0) == 0)
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

                auto parseIndex = [&](const std::string &s)
                {
                    return std::stoi(s.substr(0, s.find('/'))) - 1;
                };

                uint32_t i1 = parseIndex(a);
                uint32_t i2 = parseIndex(b);
                uint32_t i3 = parseIndex(c);

                auto addVert = [&](uint32_t idx)
                {
                    Vertex v;
                    v.pos = positions[idx];
                    v.col = color;
                    meshIndeces.push_back(meshVertices.size());
                    meshVertices.push_back(v);
                };

                addVert(i1);
                addVert(i2);
                addVert(i3);
            }
        }
    }

    Mesh objMesh;
    // should vkcheck
    objMesh.init(vulkanContext, &meshVertices, &meshIndeces);

    meshes.push_back(objMesh);
}

// Should be in Mesh.cpp
void Scene::updateModel(int modelId, glm::mat4 newModel)
{
    if (modelId >= meshes.size())
    {
        return;
    }

    meshes[modelId].setModel(newModel);
}

void Scene::tick(ve_time dt)
{
    for (Vehicle &vehicle : vehicles)
    {
        updateModel(vehicle.bodyMeshIndex, vehicle.bodyMat);
        updateModel(vehicle.wheelFLMeshIndex, vehicle.wheelFLMat);
        updateModel(vehicle.wheelFRMeshIndex, vehicle.wheelFRMat);
        updateModel(vehicle.wheelBLMeshIndex, vehicle.wheelBLMat);
        updateModel(vehicle.wheelBRMeshIndex, vehicle.wheelBRMat);

        vehicle.update(dt);
    }

    if(isCameraFollowingVehicle)
        cameraFollowVehicle(dt);

    Camera::update();
}

void Scene::setCameraFollowVehicle(uint32_t vehicleIndex)
{
    isCameraFollowingVehicle = true;
    cameraFollowedVehicleIndex = vehicleIndex;
}

void Scene::unsetCameraFollowVehicle(uint32_t vehicleIndex)
{
    isCameraFollowingVehicle = false;
}

void Scene::cameraFollowVehicle(ve_time dt)
{
    static float cameraRot = -180.0f;
    if (Input::isDown(VE_KEY_LEFT))
    {
        cameraRot += dt * 90;
    }
    if (Input::isDown(VE_KEY_RIGHT))
    {
        cameraRot -= dt * 90;
    }

    float distance = 8.0f;
    float height = 3.0f;

    float camX = vehicles[cameraFollowedVehicleIndex].getPosition().x + sin(glm::radians(cameraRot)) * distance;
    float camZ = vehicles[cameraFollowedVehicleIndex].getPosition().z + cos(glm::radians(cameraRot)) * distance;
    float camY = vehicles[cameraFollowedVehicleIndex].getPosition().y + height;

    Camera::move({camX, camY, camZ});

    glm::vec3 dir = glm::normalize(glm::vec3(vehicles[cameraFollowedVehicleIndex].getPosition().x, vehicles[cameraFollowedVehicleIndex].getPosition().y, vehicles[cameraFollowedVehicleIndex].getPosition().z) - glm::vec3(camX, camY, camZ));
    float pitch = glm::degrees(asin(dir.y));
    float yaw = glm::degrees(atan2(dir.z, dir.x));
    Camera::rotate({pitch, yaw, 0});
}

Scene::~Scene()
{
    for (auto &mesh : meshes)
        mesh.destroyBuffers(vulkanContext.device);
}
