// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"
#include "Mesh.hpp"

class Scene
{
public:
    Scene(VulkanContext newVulkanContext);
    std::vector<Mesh> meshes;
    std::vector<Vehicle> vehicles;

    void loadFile(std::string filename, glm::vec3 color);

    VulkanContext vulkanContext;

    void updateModel(int modelId, glm::mat4 newModel);

    ~Scene();
private:
};