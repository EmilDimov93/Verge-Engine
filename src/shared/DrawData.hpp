// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

#include <vector>

namespace VE
{

    struct Vertex
    {
        glm::vec3 pos;
        color_t col;
        glm::vec2 tex;

        Vertex(const glm::vec3 &position = glm::vec3(0.0f), const color_t &color = color_t(1.0f), const glm::vec2 &texture = glm::vec2(0.0f)) : pos(position), col(color), tex(texture) {}
    };

    class Mesh
    {
    public:
        Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const std::string &textureFilePath) : vertices(vertices), indices(indices), textureFilePath(textureFilePath) {}

        const std::vector<Vertex> &getVertices() const { return vertices; }
        const std::vector<uint32_t> &getIndices() const { return indices; }
        const std::string &getTextureFilePath() const { return textureFilePath; }

        static inline const std::string NO_TEXTURE = "";

    private:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::string textureFilePath;
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

        const color_t backgroundColor;

        const bool modelRemovedThisFrame;

        DrawData(const std::vector<Model> &models,
                 const std::vector<ModelInstance> &modelInstances,
                 const glm::mat4 viewMat,
                 const color_t backgroundColor,
                 const bool modelRemovedThisFrame)
            : models(models), modelInstances(modelInstances), viewMat(viewMat), backgroundColor(backgroundColor), modelRemovedThisFrame(modelRemovedThisFrame) {}
    };

}