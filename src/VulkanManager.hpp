// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>

#include "LogManager.hpp"
#include "definitions.hpp"

#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 pos;
};

class VulkanManager
{
public:
    void initVulkan(GLFWwindow *window, Size windowSize, LogManager *logRef);

    void drawFrame();

    void vkCheck(VkResult res, ErrorCode errorCode);

    void cleanup();

private:
    int currentFrame = 0;

    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    uint32_t graphicsQueueFamilyIndex;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;

    VkRenderPass renderPass;

    VkCommandPool graphicsCommandPool;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> drawFences;
    
    LogManager *log;

    void createInstance();
    void createSurface(GLFWwindow *window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(Size windowSize);
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSemaphores();

    void recordCommands();

    VkShaderModule createShaderModule(const std::vector<char> &code);
    int rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
};

class Mesh
{
public:
    Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, std::vector<Vertex> * vertices);

    int getVertexCount();

    VkBuffer getVertexBuffer();

    void destroyVertexBuffer();

private:
    int vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkBuffer createVertexBuffer(std::vector<Vertex> * vertices);

    uint32_t findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties);
};