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
    Mesh(MeshId newId, const std::vector<Vertex> &newVertices, const std::vector<uint32_t> &newIndices) : id(newId), vertices(newVertices), indices(newIndices) {}

    void update(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
    {
        this->vertices = vertices;
        this->indices = indices;

        version++;
    }

    MeshId getId() const { return id; }
    uint64_t getVersion() const { return version; };
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }

private:
    MeshId id;

    uint64_t version = 1;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct MeshInstance
{
    MeshInstanceId id;

    MeshId meshId;

    glm::mat4 model;
};

struct DrawData
{
    const std::vector<Mesh> &meshes;
    const std::vector<MeshInstance> &meshInstances;

    glm::mat4 projectionM;
    glm::mat4 viewM;

    ve_color_t backgroundColor;

    DrawData(const std::vector<Mesh> &newMeshes,
             const std::vector<MeshInstance> &newMeshInstances,
             glm::mat4 newProjectionM,
             glm::mat4 newViewM,
             ve_color_t newBackgroundColor)
        : meshes(newMeshes), meshInstances(newMeshInstances), projectionM(newProjectionM), viewM(newViewM), backgroundColor(newBackgroundColor) {}
};