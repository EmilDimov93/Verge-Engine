// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "ModelLoader.hpp"

#include "../shared/Log.hpp"

#include <fstream>
#include <filesystem>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../../ext/stb_image/stb_image.h"

#define CGLTF_IMPLEMENTATION
#include "../../ext/cgltf/cgltf.h"

namespace VE
{
    [[nodiscard]] std::pair<std::vector<Mesh>, std::vector<Material>> loadOBJ(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            return {};

        std::vector<glm::vec3> positions;
        std::unordered_map<std::string, Material> materials;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;

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
                    materials[currentMat].baseColor.a = 1.f;
                }
                else if (line.starts_with("Kd ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(3));
                    glm::vec3 kd;
                    ss >> kd.r >> kd.g >> kd.b;
                    materials[currentMat].baseColor = srgbToLinear(color_t(kd, materials[currentMat].baseColor.a));
                }
                else if (line.starts_with("map_Kd ") && !currentMat.empty())
                {
                    std::string texturePath = line.substr(7);
                    trim(texturePath);
                    std::filesystem::path resolvedPath = std::filesystem::path(mtlPath).parent_path() / texturePath;

                    int width = 0;
                    int height = 0;
                    int channelsInFile = 0;

                    stbi_uc *decodedPixels = stbi_load(resolvedPath.string().c_str(), &width, &height, &channelsInFile, STBI_rgb_alpha);

                    if (decodedPixels)
                    {
                        materials[currentMat].textureSize.x = static_cast<uint32_t>(width);
                        materials[currentMat].textureSize.y = static_cast<uint32_t>(height);

                        materials[currentMat].texturePixels.assign(decodedPixels, decodedPixels + static_cast<size_t>(width) * height * 4);

                        stbi_image_free(decodedPixels);
                    }
                    else
                    {
                        Log::add('R', 111);
                    }
                }
                else if (line.starts_with("d ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(2));
                    float dissolve = 1.f;
                    ss >> dissolve;
                    materials[currentMat].baseColor.a = dissolve;
                }
                else if (line.starts_with("Tr ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(3));
                    float transparency = 0.f;
                    ss >> transparency;
                    materials[currentMat].baseColor.a = 1.f - transparency;
                }
                else if (line.starts_with("Pm ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(3));
                    float metallic = 0.f;
                    ss >> metallic;
                    materials[currentMat].metallic = metallic;
                }
                else if (line.starts_with("Pr ") && !currentMat.empty())
                {
                    std::stringstream ss(line.substr(3));
                    float roughness = 1.f;
                    ss >> roughness;
                    materials[currentMat].roughness = roughness;
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
                materialList.push_back(Material());
                currentMaterialIndex = resolvedMaterialIndex;
            }

            Mesh newMesh(currentMeshVertices, currentMeshIndices, resolvedMaterialIndex);
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
                        materialList.push_back(it->second);
                        materialIndexByName.emplace(mat, currentMaterialIndex);
                    }
                    else
                    {
                        currentMaterialIndex = idxIt->second;
                    }
                }
            }
            else if (line.starts_with("v "))
            {
                glm::vec3 p;
                std::stringstream ss(line.substr(2));
                ss >> p.x >> p.y >> p.z;
                positions.push_back(p);
            }
            else if (line.starts_with("vn "))
            {
                glm::vec3 n;
                std::stringstream ss(line.substr(3));
                ss >> n.x >> n.y >> n.z;
                normals.push_back(n);
            }
            else if (line.starts_with("f "))
            {
                std::stringstream ss(line.substr(2));
                std::string a, b, c;
                ss >> a >> b >> c;

                auto parseIndices = [](const std::string &token) -> std::tuple<uint32_t, int32_t, int32_t>
                {
                    size_t firstSlash = token.find('/');
                    uint32_t positionIndex = static_cast<uint32_t>(std::stoi(token.substr(0, firstSlash)) - 1);
                    int32_t texCoordIndex = -1;
                    int32_t normalIndex = -1;

                    if (firstSlash != std::string::npos)
                    {
                        size_t secondSlash = token.find('/', firstSlash + 1);
                        if (firstSlash + 1 < token.size() && token[firstSlash + 1] != '/')
                            texCoordIndex = std::stoi(token.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;

                        if (secondSlash != std::string::npos && secondSlash + 1 < token.size())
                            normalIndex = std::stoi(token.substr(secondSlash + 1)) - 1;
                    }

                    return {positionIndex, texCoordIndex, normalIndex};
                };

                std::tuple<uint32_t, int32_t, int32_t> parsed[3] = {parseIndices(a), parseIndices(b), parseIndices(c)};

                glm::vec3 faceNormal = glm::normalize(glm::cross(positions[std::get<0>(parsed[1])] - positions[std::get<0>(parsed[0])],
                                                                 positions[std::get<0>(parsed[2])] - positions[std::get<0>(parsed[0])]));

                for (auto &[positionIndex, texCoordIndex, normalIndex] : parsed)
                {
                    Vertex vertex;
                    vertex.pos = positions[positionIndex];
                    vertex.tex = (texCoordIndex >= 0 && texCoordIndex < static_cast<int32_t>(texCoords.size())) ? texCoords[texCoordIndex] : glm::vec2(0.f);
                    vertex.tex.y = 1.f - vertex.tex.y;
                    vertex.norm = (normalIndex >= 0 && normalIndex < static_cast<int32_t>(normals.size())) ? normals[normalIndex] : faceNormal;

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

        return std::make_pair(std::move(meshes), std::move(materialList));
    }

    [[nodiscard]] std::pair<std::vector<Mesh>, std::vector<Material>> loadGLB(const std::string &filePath)
    {
        cgltf_options options = {};
        cgltf_data *data = nullptr;

        if (cgltf_parse_file(&options, filePath.c_str(), &data) != cgltf_result_success)
            return {};

        if (cgltf_load_buffers(&options, data, filePath.c_str()) != cgltf_result_success)
        {
            cgltf_free(data);
            return {};
        }

        std::vector<Mesh> meshes;
        std::vector<Material> materialList;

        const std::filesystem::path baseDir = std::filesystem::path(filePath).parent_path();

        materialList.reserve(data->materials_count);
        for (cgltf_size materialIndex = 0; materialIndex < data->materials_count; ++materialIndex)
        {
            const cgltf_material &source = data->materials[materialIndex];
            Material material;

            if (source.has_pbr_metallic_roughness)
            {
                const cgltf_pbr_metallic_roughness &pbr = source.pbr_metallic_roughness;

                material.baseColor = color_t(glm::vec3(pbr.base_color_factor[0],
                                                       pbr.base_color_factor[1],
                                                       pbr.base_color_factor[2]),
                                             pbr.base_color_factor[3]);
                material.metallic = pbr.metallic_factor;
                material.roughness = pbr.roughness_factor;

                const cgltf_texture *baseColorTexture = pbr.base_color_texture.texture;
                if (baseColorTexture && baseColorTexture->image)
                {
                    const cgltf_image *image = baseColorTexture->image;

                    int width = 0;
                    int height = 0;
                    int channelsInFile = 0;
                    stbi_uc *decodedPixels = nullptr;

                    if (image->uri)
                    {
                        std::string fullPath = (baseDir / image->uri).string();

                        decodedPixels = stbi_load(fullPath.c_str(), &width, &height, &channelsInFile, STBI_rgb_alpha);
                    }
                    else if (image->buffer_view)
                    {
                        const cgltf_buffer_view *view = image->buffer_view;
                        const uint8_t *encodedBytes = static_cast<const uint8_t *>(view->buffer->data) + view->offset;
                        const int encodedSize = static_cast<int>(view->size);

                        decodedPixels = stbi_load_from_memory(encodedBytes, encodedSize, &width, &height, &channelsInFile, STBI_rgb_alpha);
                    }

                    if (decodedPixels)
                    {
                        material.textureSize.x = static_cast<uint32_t>(width);
                        material.textureSize.y = static_cast<uint32_t>(height);

                        material.texturePixels.assign(decodedPixels,
                                                      decodedPixels + static_cast<size_t>(width) * height * 4);

                        stbi_image_free(decodedPixels);
                    }
                    else
                    {
                        Log::add('R', 111);
                    }
                }
            }

            materialList.push_back(material);
        }

        const uint32_t defaultMaterialIndex = static_cast<uint32_t>(materialList.size());
        bool defaultMaterialUsed = false;

        for (cgltf_size meshIndex = 0; meshIndex < data->meshes_count; ++meshIndex)
        {
            const cgltf_mesh &sourceMesh = data->meshes[meshIndex];

            for (cgltf_size primitiveIndex = 0; primitiveIndex < sourceMesh.primitives_count; ++primitiveIndex)
            {
                const cgltf_primitive &primitive = sourceMesh.primitives[primitiveIndex];

                if (primitive.type != cgltf_primitive_type_triangles)
                    continue;

                const cgltf_accessor *positionAccessor = nullptr;
                const cgltf_accessor *normalAccessor = nullptr;
                const cgltf_accessor *texCoordAccessor = nullptr;

                for (cgltf_size attributeIndex = 0; attributeIndex < primitive.attributes_count; ++attributeIndex)
                {
                    const cgltf_attribute &attribute = primitive.attributes[attributeIndex];
                    switch (attribute.type)
                    {
                    case cgltf_attribute_type_position:
                        positionAccessor = attribute.data;
                        break;
                    case cgltf_attribute_type_normal:
                        normalAccessor = attribute.data;
                        break;
                    case cgltf_attribute_type_texcoord:
                        if (attribute.index == 0)
                            texCoordAccessor = attribute.data;
                        break;
                    default:
                        break;
                    }
                }

                if (!positionAccessor)
                    continue;

                const cgltf_size vertexCount = positionAccessor->count;

                std::vector<Vertex> meshVertices(vertexCount);
                for (cgltf_size vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
                {
                    Vertex &vertex = meshVertices[vertexIndex];

                    float position[3] = {0.f, 0.f, 0.f};
                    cgltf_accessor_read_float(positionAccessor, vertexIndex, position, 3);
                    vertex.pos = glm::vec3(position[0], position[1], position[2]);

                    if (normalAccessor)
                    {
                        float normal[3] = {0.f, 0.f, 0.f};
                        cgltf_accessor_read_float(normalAccessor, vertexIndex, normal, 3);
                        vertex.norm = glm::vec3(normal[0], normal[1], normal[2]);
                    }
                    else
                    {
                        vertex.norm = glm::vec3(0.f);
                    }

                    if (texCoordAccessor)
                    {
                        float texCoord[2] = {0.f, 0.f};
                        cgltf_accessor_read_float(texCoordAccessor, vertexIndex, texCoord, 2);
                        vertex.tex = glm::vec2(texCoord[0], texCoord[1]);
                    }
                    else
                    {
                        vertex.tex = glm::vec2(0.f);
                    }
                }

                std::vector<uint32_t> meshIndices;
                if (primitive.indices)
                {
                    const cgltf_size indexCount = primitive.indices->count;
                    meshIndices.resize(indexCount);
                    for (cgltf_size i = 0; i < indexCount; ++i)
                        meshIndices[i] = static_cast<uint32_t>(cgltf_accessor_read_index(primitive.indices, i));
                }
                else
                {
                    meshIndices.resize(vertexCount);
                    for (cgltf_size i = 0; i < vertexCount; ++i)
                        meshIndices[i] = static_cast<uint32_t>(i);
                }

                if (!normalAccessor)
                {
                    for (size_t i = 0; i + 2 < meshIndices.size(); i += 3)
                    {
                        Vertex &v0 = meshVertices[meshIndices[i]];
                        Vertex &v1 = meshVertices[meshIndices[i + 1]];
                        Vertex &v2 = meshVertices[meshIndices[i + 2]];
                        glm::vec3 faceNormal = glm::normalize(glm::cross(v1.pos - v0.pos, v2.pos - v0.pos));
                        v0.norm = faceNormal;
                        v1.norm = faceNormal;
                        v2.norm = faceNormal;
                    }
                }

                uint32_t resolvedMaterialIndex;
                if (primitive.material)
                {
                    resolvedMaterialIndex = static_cast<uint32_t>(primitive.material - data->materials);
                }
                else
                {
                    resolvedMaterialIndex = defaultMaterialIndex;
                    defaultMaterialUsed = true;
                }

                meshes.emplace_back(meshVertices, meshIndices, resolvedMaterialIndex);
            }
        }

        if (defaultMaterialUsed)
            materialList.push_back(Material());

        cgltf_free(data);

        return std::make_pair(std::move(meshes), std::move(materialList));
    }

    std::pair<std::vector<Mesh>, std::vector<Material>> loadModel(const std::string &filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();

        std::pair<std::vector<Mesh>, std::vector<Material>> data;

        if (ext == ".obj")
        {
            data = loadOBJ(filePath);
        }
        else if (ext == ".glb")
        {
            data = loadGLB(filePath);
        }
        else
        {
            Log::add('E', 101);
            return data;
        }

        return data;
    }
}