// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

#include <vector>

struct Vertex
{
    glm::vec3 pos;
    ve_color_t col;

    Vertex(const glm::vec3 &position = glm::vec3(0.0f), const ve_color_t &color = ve_color_t(1.0f)) : pos(position), col(color) {}
};

class Mesh
{
public:
    Mesh(MeshHandle handle, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) : handle(handle), vertices(vertices), indices(indices) {}

    void update(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    {
        this->vertices = vertices;
        this->indices = indices;

        version++;
    }

    MeshHandle getHandle() const { return handle; }
    uint64_t getVersion() const { return version; };
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }

private:
    const MeshHandle handle;

    uint64_t version = 1;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct MeshInstance
{
    const MeshInstanceHandle handle;

    MeshHandle meshHandle;

    glm::mat4 modelMat;

    MeshInstance(MeshInstanceHandle handle, MeshHandle meshHandle, glm::mat4 modelMat)
    : handle(handle), meshHandle(meshHandle), modelMat(modelMat) { }
};

struct DrawData
{
    const std::vector<Mesh> &meshes;
    const std::vector<MeshInstance> &meshInstances;

    glm::mat4 projectionMat;
    glm::mat4 viewMat;

    ve_color_t backgroundColor;

    DrawData(const std::vector<Mesh> &meshes,
             const std::vector<MeshInstance> &meshInstances,
             glm::mat4 projectionMat,
             glm::mat4 viewMat,
             ve_color_t backgroundColor)
        : meshes(meshes), meshInstances(meshInstances), projectionMat(projectionMat), viewMat(viewMat), backgroundColor(backgroundColor) {}
};