// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Mesh.hpp"
#include "definitions.hpp"

struct GLFWwindow;

class Log;
class ErrorCode;

class VulkanManager
{
public:
    VulkanManager(GLFWwindow* window, Size2 windowSize);

    void updateModel(int modelId, glm::mat4 newModel);

    void drawFrame();

    void vkCheck(VkResult res, ErrorCode errorCode);

    void loadFile(std::string filename, glm::vec3 color);

    ~VulkanManager();

private:
    int currentFrame = 0;

    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    int32_t graphicsQueueFamilyIndex;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    VkImage depthBufferImage;
    VkDeviceMemory depthBufferImageMemory;
    VkImageView depthBufferImageView;
    VkFormat depthFormat;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPushConstantRange pushConstantRange;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkBuffer> vpUniformBuffer;
    std::vector<VkDeviceMemory> vpUniformBufferMemory;

    std::vector<VkBuffer> modelDUniformBuffer;
    std::vector<VkDeviceMemory> modelDUniformBufferMemory;

    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;

    VkRenderPass renderPass;

    VkCommandPool graphicsCommandPool;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> drawFences;

    std::vector<Mesh> meshes;

    struct UboViewProjection
    {
        glm::mat4 projection;
        glm::mat4 view;
    }uboViewProjection;

    void createInstance();
    void createSurface(GLFWwindow* window);
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

    void recordCommands(uint32_t currentImage);

    void updateUniformBuffers(uint32_t imageIndex);

    VkShaderModule createShaderModule(const std::vector<char> &code);
    int rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
};