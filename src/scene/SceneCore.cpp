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
    surfaces.push_back({1.0f, {0.1f, 0.1f, 0.1f}});
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

    Mesh objMesh(newMeshHandle, meshVertices, meshIndices);

    meshes.push_back(objMesh);

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
