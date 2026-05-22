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

    static const std::vector<Mesh> loadOBJ(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            static const std::vector<Mesh> emptyVector;
            return emptyVector;
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
                    materials[currentMat].diffuseColor = color_t(kd, 1.0f);
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

        return meshes;
    }
}