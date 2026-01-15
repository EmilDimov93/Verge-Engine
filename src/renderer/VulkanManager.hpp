// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../shared/Mesh.hpp"
#include "../shared/definitions.hpp"

#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow;

class Log;
class ErrorCode;

class VulkanManager
{
public:
    VulkanManager(GLFWwindow *window, Size2 windowSize);

    void drawFrame(DrawData drawData);

    void vkCheck(VkResult res, ErrorCode errorCode);

    ~VulkanManager();

private:
    struct MeshGPU
    {
        MeshHandle handle;

        uint64_t version = 0;

        uint64_t vertexCount = 0;
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

        uint64_t indexCount = 0;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    };

    std::vector<MeshGPU> MeshGPUs;
    void createVertexBuffer(MeshGPU &MeshGPU, const std::vector<Vertex> &vertices);
    void createIndexBuffer(MeshGPU &MeshGPU, const std::vector<uint32_t> &indices);
    void initMeshGPU(const Mesh& mesh);
    void updateMeshGPU(MeshGPU& MeshGPU, const Mesh& mesh);

    int currentFrame = 0;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    int32_t graphicsQueueFamilyIndex = -1;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    VkImage depthBufferImage = VK_NULL_HANDLE;
    VkDeviceMemory depthBufferImageMemory = VK_NULL_HANDLE;
    VkImageView depthBufferImageView = VK_NULL_HANDLE;
    VkFormat depthFormat;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPushConstantRange pushConstantRange;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkBuffer> vpUniformBuffer;
    std::vector<VkDeviceMemory> vpUniformBufferMemory;

    std::vector<VkBuffer> modelDUniformBuffer;
    std::vector<VkDeviceMemory> modelDUniformBufferMemory;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    VkRenderPass renderPass = VK_NULL_HANDLE;

    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> drawFences;

    void createInstance();
    void createSurface(GLFWwindow *window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(Size2 windowSize);
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createPushConstantRange();
    void createGraphicsPipeline();
    void createDepthBufferImage();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSemaphores();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();

    void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory);

    void recordCommands(uint32_t currentImage, const std::vector<Mesh> &meshes, const std::vector<MeshInstance> &meshInstances, ve_color_t backgroundColor);

    void updateUniformBuffers(uint32_t imageIndex, glm::mat4 projectionM, glm::mat4 viewM);

    VkShaderModule createShaderModule(const std::vector<char> &code);
    int rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
};