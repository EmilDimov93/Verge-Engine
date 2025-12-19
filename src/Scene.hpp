// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "Vehicle.hpp"
#include "Mesh.hpp"

class Scene
{
public:
    std::vector<Mesh> meshes;
    std::vector<Vehicle> vehicles;

    Scene(VulkanContext newVulkanContext);

    void addVehicle(const VE_STRUCT_VEHICLE_CREATE_INFO &info);

    void loadFile(std::string filename, glm::vec3 color);

    VulkanContext vulkanContext;

    void updateModel(int modelId, glm::mat4 newModel);

    void tick(ve_time dt);

    ~Scene();
private:
};