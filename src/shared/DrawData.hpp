// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

#include <vector>

struct Vertex
{
    glm::vec3 pos;
    ve_color_t col;
    glm::vec2 tex;

    Vertex(const glm::vec3 &position = glm::vec3(0.0f), const ve_color_t &color = ve_color_t(1.0f)) : pos(position), col(color) {}
};

class Mesh
{
public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) : vertices(vertices), indices(indices) {}

    const std::vector<Vertex> &getVertices() const { return vertices; }
    const std::vector<uint32_t> &getIndices() const { return indices; }

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

class Model
{
public:
    Model(ModelHandle handle, const std::vector<Mesh> &meshes) : handle(handle), meshes(meshes) {}

    void update(const std::vector<Mesh> &meshes)
    {
        this->meshes = meshes;

        version++;
    }

    ModelHandle getHandle() const { return handle; };
    uint64_t getVersion() const { return version; }
    const std::vector<Mesh> &getMeshes() const { return meshes; }

private:
    ModelHandle handle;

    uint64_t version = 1;

    std::vector<Mesh> meshes;
};

struct ModelInstance
{
    ModelInstanceHandle handle;

    ModelHandle modelHandle;

    glm::mat4 modelMat;

    ModelInstance(ModelInstanceHandle handle, ModelHandle modelHandle, glm::mat4 modelMat)
        : handle(handle), modelHandle(modelHandle), modelMat(modelMat) {}
};

struct DrawData
{
    const std::vector<Model> &models;
    const std::vector<ModelInstance> &modelInstances;

    const glm::mat4 viewMat;

    const ve_color_t backgroundColor;

    const bool modelRemovedThisFrame;

    DrawData(const std::vector<Model> &models,
             const std::vector<ModelInstance> &modelInstances,
             const glm::mat4 viewMat,
             const ve_color_t backgroundColor,
             const bool modelRemovedThisFrame)
        : models(models), modelInstances(modelInstances), viewMat(viewMat), backgroundColor(backgroundColor), modelRemovedThisFrame(modelRemovedThisFrame) {}
};