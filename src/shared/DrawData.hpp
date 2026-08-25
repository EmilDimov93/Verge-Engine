// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "definitions.hpp"

#include <vector>

namespace VE
{
    static constexpr uint32_t INVALID_TEXTURE_INDEX = 0;

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 tex;
        glm::vec3 norm;

        Vertex(const glm::vec3 &position = glm::vec3(0.f), const glm::vec2 &texture = glm::vec2(0.f), const glm::vec3 &normal = glm::vec3(0.f, 1.f, 0.f)) : pos(position), tex(texture), norm(normal) {}
    };

    struct Material
    {
        color_t baseColor;
        float metallic;
        float roughness;

        // GPU
        uint32_t texIndex = INVALID_TEXTURE_INDEX;

        // CPU
        std::vector<uint8_t> texturePixels;
        glm::uvec2 textureSize{};

        Material(color_t baseColor = color_t(1.f), float metallic = 0.f, float roughness = 1.f) : baseColor(baseColor), metallic(metallic), roughness(roughness) {}
    };

    class Mesh
    {
    public:
        Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, uint32_t materialIndex) : vertices(vertices), indices(indices), materialIndex(materialIndex) {}

        [[nodiscard]] const std::vector<Vertex> &getVertices() const { return vertices; }
        [[nodiscard]] const std::vector<uint32_t> &getIndices() const { return indices; }
        [[nodiscard]] uint32_t getMaterialIndex() const { return materialIndex; }

    private:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialIndex;
    };

    class Model
    {
    public:
        Model(ModelHandle handle, const std::vector<Mesh> &meshes, const std::vector<Material> &materials) : handle(handle), meshes(meshes), materials(materials) {}

        void update(const std::vector<Mesh> &meshes)
        {
            this->meshes = meshes;

            version++;
        }

        [[nodiscard]] ModelHandle getHandle() const { return handle; };
        [[nodiscard]] uint64_t getVersion() const { return version; }
        [[nodiscard]] const std::vector<Mesh> &getMeshes() const { return meshes; }
        [[nodiscard]] const std::vector<Material> &getMaterials() const { return materials; }

    private:
        ModelHandle handle;

        uint64_t version = 1;

        std::vector<Mesh> meshes;
        std::vector<Material> materials;
    };

    struct ModelInstance
    {
        ModelInstanceHandle handle;

        ModelHandle modelHandle;

        glm::mat4 modelMat;

        float lightIntensity = 0.f;
        color_t lightColor = color_t(1.f);

        ModelInstance(ModelInstanceHandle handle, ModelHandle modelHandle, glm::mat4 modelMat, float lightIntensity = 0.f, color_t lightColor = color_t(1.f))
            : handle(handle), modelHandle(modelHandle), modelMat(modelMat), lightIntensity(lightIntensity), lightColor(lightColor) {}
    };

    class Widget
    {
    public:
        Widget(WidgetHandle handle, const std::vector<Mesh> &meshes, const std::vector<Material> &materials) : handle(handle), meshes(meshes), materials(materials) {}

        void update(const std::vector<Mesh> &meshes)
        {
            this->meshes = meshes;

            version++;
        }

        [[nodiscard]] WidgetHandle getHandle() const { return handle; };
        [[nodiscard]] uint64_t getVersion() const { return version; }
        [[nodiscard]] const std::vector<Mesh> &getMeshes() const { return meshes; }
        [[nodiscard]] const std::vector<Material> &getMaterials() const { return materials; }

    private:
        WidgetHandle handle;

        uint64_t version = 1;

        std::vector<Mesh> meshes;
        std::vector<Material> materials;
    };

    struct WidgetInstance
    {
        WidgetInstanceHandle handle;

        WidgetHandle widgetHandle;

        glm::vec2 coords;

        float uniformScale;

        WidgetInstance(WidgetInstanceHandle handle, WidgetHandle widgetHandle, glm::vec2 coords, float uniformScale)
            : handle(handle), widgetHandle(widgetHandle), coords(coords), uniformScale(uniformScale) {}
    };

    struct SceneDrawData
    {
        const std::vector<Model> &models;
        const std::vector<ModelInstance> &modelInstances;

        const glm::mat4 viewMat;

        const color_t backgroundColor;

        const float outdoorBrightness;

        const bool modelRemovedThisFrame;

        SceneDrawData(const std::vector<Model> &models,
                      const std::vector<ModelInstance> &modelInstances,
                      const glm::mat4 viewMat,
                      const color_t backgroundColor,
                      const float outdoorBrightness,
                      const bool modelRemovedThisFrame)
            : models(models), modelInstances(modelInstances), viewMat(viewMat), backgroundColor(backgroundColor), outdoorBrightness(outdoorBrightness), modelRemovedThisFrame(modelRemovedThisFrame) {}
    };

    struct UIDrawData
    {
        const std::vector<Widget> &widgets;
        const std::vector<WidgetInstance> &widgetInstances;

        const bool isValid;

        UIDrawData()
            : widgets(emptyWidgets), widgetInstances(emptyInstances), isValid(false) {}

        UIDrawData(const std::vector<Widget> &widgets,
                   const std::vector<WidgetInstance> &widgetInstances)
            : widgets(widgets), widgetInstances(widgetInstances), isValid(true) {}

    private:
        static inline const std::vector<Widget> emptyWidgets{};
        static inline const std::vector<WidgetInstance> emptyInstances{};
    };

    struct PostEffects
    {
        float vignetteStrength = 0.f;
        float vignetteRadius = 1.f;
        float exposure = 1.f;
        float fxaa = false;
        bool dithering = false;
    };
}