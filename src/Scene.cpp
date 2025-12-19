// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Scene.hpp"

#include <sstream>
#include <vector>

Scene::Scene(VulkanContext newVulkanContext)
{
    vulkanContext = newVulkanContext;
}

void Scene::addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info)
{
    Vehicle newVehicle;
    newVehicle.init(info);
    vehicles.push_back(newVehicle);
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
    for(Vehicle &vehicle : vehicles){
        updateModel(vehicle.bodyMeshIndex, vehicle.bodyMat);
        updateModel(vehicle.wheelFLMeshIndex, vehicle.wheelFLMat);
        updateModel(vehicle.wheelFRMeshIndex, vehicle.wheelFRMat);
        updateModel(vehicle.wheelBLMeshIndex, vehicle.wheelBLMat);
        updateModel(vehicle.wheelBRMeshIndex, vehicle.wheelBRMat);

        vehicle.update(dt);
    }
}

Scene::~Scene()
{
    for (auto &mesh : meshes)
        mesh.destroyBuffers(vulkanContext.device);
}
