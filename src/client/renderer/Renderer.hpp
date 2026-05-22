// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/DrawData.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <mutex>

namespace VE
{
    class ErrorCode;

    static constexpr uint32_t INVALID_TEXTURE_INDEX = 0;

    class Renderer
    {
    public:
        Renderer(GLFWwindow *window, Size2 windowSize);

        void drawFrame(const SceneDrawData &sceneDrawData, const UIDrawData &uiDrawData, const glm::mat4 projectionMat);

        ~Renderer();

        void markFramebufferResized() { framebufferResized = true; };

    private:
        static constexpr uint32_t MAX_FRAME_DRAWS = 2;
        
        static constexpr Size2 SHADOW_MAP_EXTENT = {4096, 4096};

        static constexpr uint32_t TEXTURE_SAMPLER_POOL_CHUNK_SIZE = 1e3;

        struct MeshBuffer
        {
            uint32_t vertexCount = 0;
            VkBuffer vertexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

            uint32_t indexCount = 0;
            VkBuffer indexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

            uint32_t texIndex = INVALID_TEXTURE_INDEX;
        };

        struct ModelBuffer
        {
            ModelHandle handle;

            std::vector<MeshBuffer> meshBuffers;

            uint32_t version = 0;

            ModelBuffer(ModelHandle handle) : handle(handle) {}
        };

        std::vector<ModelBuffer> modelBuffers;

        struct UboCamera
        {
            glm::mat4 projection;
            glm::mat4 view;
            glm::mat4 lightSpaceMat;
        };
        std::vector<VkBuffer> cameraUniformBuffer;
        std::vector<VkDeviceMemory> cameraUniformBufferMemory;

        struct UboLighting
        {
            glm::vec4 lightPos;
            glm::vec3 lightColor;
            glm::vec4 viewPos;
        };
        std::vector<VkBuffer> lightingUniformBuffer;
        std::vector<VkDeviceMemory> lightingUniformBufferMemory;

        struct PushData
        {
            glm::mat4 model;
            uint32_t textureIndex;
            float lightStrength;
        };

        struct ShadowPushData
        {
            glm::mat4 model;
            glm::mat4 lightSpaceMat;
        };

        GLFWwindow *window = nullptr;

        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;

        uint32_t graphicsQueueFamilyIndex = 0;
        uint32_t transferQueueFamilyIndex = 0;

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffers;

        VkImage depthBufferImage = VK_NULL_HANDLE;
        VkDeviceMemory depthBufferImageMemory = VK_NULL_HANDLE;
        VkImageView depthBufferImageView = VK_NULL_HANDLE;
        VkFormat depthFormat;

        VkPipeline shadowPipeline = VK_NULL_HANDLE;
        VkPipelineLayout shadowPipelineLayout = VK_NULL_HANDLE;

        VkImage shadowDepthBufferImage = VK_NULL_HANDLE;
        VkDeviceMemory shadowDepthBufferImageMemory = VK_NULL_HANDLE;
        VkImageView shadowDepthBufferImageView = VK_NULL_HANDLE;
        VkFormat shadowDepthFormat;
        VkSampler shadowSampler = VK_NULL_HANDLE;

        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets;

        VkDescriptorSetLayout samplerSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorPool> samplerDescriptorPools;
        std::vector<VkDescriptorSet> samplerDescriptorSets;

        VkSampler textureSampler;
        std::vector<VkImage> textureImages;
        std::vector<VkDeviceMemory> textureImageMemory;
        std::vector<VkImageView> textureImageViews;

        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> drawFences;

        std::mutex graphicsQueueMutex;
        mutable std::mutex transferQueueMutex;
        std::recursive_mutex modelMutex;
        std::mutex textureMutex;

        uint32_t currentFrame = 0;
        bool framebufferResized = false;

        // Init
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();

        void createCommandPool();
        void createCommandBuffers();

        void createSwapChain(Size2 windowSize);
        void createImageViews();

        void findDepthFormat();
        void createShadowSampler();
        void createTextureSampler();
        
        void createDepthBufferImage();
        void createShadowDepthBufferImage();
        
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createShadowPipeline();

        void createUniformBuffers();
        void createUniformDescriptorPool();
        void createDescriptorSets();

        void createSemaphores();

        // Runtime
        void recordShadowPass(const std::vector<Model>& models, const std::vector<ModelInstance>& modelInstances, const glm::mat4& lightSpaceMat);
        void recordMainPass(uint32_t currentImage, const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, color_t backgroundColor, const glm::mat4 &lightSpaceMat);
        void updateUniformBuffers(uint32_t imageIndex, glm::mat4 projectionMat, glm::mat4 viewMat, glm::vec4 lightPos, glm::vec3 lightColor, glm::mat4 lightSpaceMat);
        void recreateSwapChain();

        // Helpers
        static void vkCheck(VkResult res, ErrorCode errorCode);
        void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory) const;
        VkResult copyBuffer(VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkFence fence) const;
        static std::vector<char> readFile(const std::string &fileName);
        VkShaderModule createShaderModule(const std::vector<char> &code) const;
        static uint32_t rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);

        // Models
        void syncModelBuffers(const std::vector<Model> &models);
        void createVertexBuffer(MeshBuffer &meshBuffer, const std::vector<Vertex> &vertices);
        void createIndexBuffer(MeshBuffer &meshBuffer, const std::vector<uint32_t> &indices);
        void initModelBuffer(const Model &model);
        void updateModelBuffer(ModelBuffer &modelBuffer, const Model &model);
        void removeOrphanedModel(const std::vector<ModelInstance> &modelInstances);
        void destroyMeshBuffer(MeshBuffer &meshBuffer);

        // Textures
        void createFallbackTexture();
        size_t createTextureImage(std::string fileName);
        size_t createTexture(std::string fileName);
        size_t createTextureDescriptor(VkImageView textureImageView);
        VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory *imageMemory);
    };

}