// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "DrawData.hpp"

#include <vector>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <sstream>

namespace VE
{

    
[[nodiscard]] static ModelData loadOBJ(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            return {};

        struct MaterialEntry
        {
            Material material{};
            std::string diffuseTexturePath;
        };

        std::vector<glm::vec3> positions;
        std::unordered_map<std::string, MaterialEntry> materials;
        std::vector<glm::vec2> texCoords;

        std::string currentTexturePath;
        uint32_t currentMaterialIndex = UINT32_MAX;

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
                    materials[currentMat].material.baseColor = color_t(kd, materials[currentMat].material.baseColor.a);
                }
                else if (line.starts_with("map_Kd ") && !currentMat.empty())
                {
                    std::string texturePath = line.substr(7);
                    trim(texturePath);
                    std::filesystem::path resolvedPath = std::filesystem::path(mtlPath).parent_path() / texturePath;
                    materials[currentMat].diffuseTexturePath = resolvedPath.string();
                }
                else if (line.starts_with("d ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(2));
                    float dissolve = 1.0f;
                    ss >> dissolve;
                    materials[currentMat].material.baseColor.a = dissolve;
                }
                else if (line.starts_with("Tr ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(3));
                    float transparency = 0.0f;
                    ss >> transparency;
                    materials[currentMat].material.baseColor.a = 1.0f - transparency;
                }
                else if (line.starts_with("Pm ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(3));
                    float metallic = 0.0f;
                    ss >> metallic;
                    materials[currentMat].material.metallic = metallic;
                }
                else if (line.starts_with("Pr ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(3));
                    float roughness = 1.0f;
                    ss >> roughness;
                    materials[currentMat].material.roughness = roughness;
                }
            }
        };

        std::vector<Mesh> meshes;
        std::vector<Material> materialList;
        std::unordered_map<std::string, uint32_t> materialIndexByName;
        auto finalizeCurrentMesh = [&]()
        {
            if (currentMeshVertices.empty())
                return;

            uint32_t resolvedMaterialIndex = currentMaterialIndex;
            if (resolvedMaterialIndex == UINT32_MAX)
            {
                resolvedMaterialIndex = static_cast<uint32_t>(materialList.size());
                materialList.push_back(Material{});
                currentMaterialIndex = resolvedMaterialIndex;
            }

            Mesh newMesh(currentMeshVertices, currentMeshIndices, resolvedMaterialIndex, currentTexturePath);
            meshes.push_back(newMesh);

            currentMeshVertices.clear();
            currentMeshIndices.clear();
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
                {
                    auto idxIt = materialIndexByName.find(mat);
                    if (idxIt == materialIndexByName.end())
                    {
                        currentMaterialIndex = static_cast<uint32_t>(materialList.size());
                        materialList.push_back(it->second.material);
                        materialIndexByName.emplace(mat, currentMaterialIndex);
                    }
                    else
                    {
                        currentMaterialIndex = idxIt->second;
                    }
                    currentTexturePath = it->second.diffuseTexturePath;
                }
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
                    vertex.tex = (texCoordIndex >= 0) ? texCoords[texCoordIndex] : glm::vec2(0.0f);
                    vertex.tex.y = 1.0f - vertex.tex.y;
                    vertex.norm = glm::normalize(glm::cross(positions[parsed[1].first] - positions[parsed[0].first],
                                                            positions[parsed[2].first] - positions[parsed[0].first]));

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

        return ModelData{std::move(meshes), std::move(materialList)};
    }
}