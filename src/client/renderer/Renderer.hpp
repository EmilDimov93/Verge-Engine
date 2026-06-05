// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../../shared/DrawData.hpp"

#include "../../shared/Log.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <mutex>

namespace VE
{
    static constexpr uint32_t INVALID_TEXTURE_INDEX = 0;

    struct GraphicsPipeline
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets;
    };

    struct ImageAttachment
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
    };

    class Renderer
    {
    public:
        Renderer(GLFWwindow *window, Size2 windowSize);

        void drawFrame(const SceneDrawData &sceneDrawData, const UIDrawData &uiDrawData, const glm::mat4 projectionMat, const PostEffects& postEffects);

        ~Renderer();

        void markFramebufferResized() { framebufferResized = true; };

    private:
        static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

        static constexpr Size2 SHADOW_MAP_EXTENT = {4096, 4096};

        static constexpr uint32_t TEXTURE_SAMPLER_POOL_CHUNK_SIZE = 1e3;

        static constexpr char PIPELINE_CACHE_FILE_NAME[] = "pipeline_cache.bin";

        struct MeshBuffer
        {
            uint32_t vertexCount = 0;
            VkBuffer vertexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

            uint32_t indexCount = 0;
            VkBuffer indexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

            uint32_t texIndex = INVALID_TEXTURE_INDEX;

            bool isTransparent = false;
        };

        struct ModelBuffer
        {
            ModelHandle handle;

            std::vector<MeshBuffer> meshBuffers;

            uint32_t version = 0;

            ModelBuffer(ModelHandle handle) : handle(handle) {}
        };

        struct WidgetBuffer
        {
            WidgetHandle handle;

            std::vector<MeshBuffer> meshBuffers;

            uint32_t version = 0;

            WidgetBuffer(WidgetHandle handle) : handle(handle) {}
        };

        struct UboCamera
        {
            glm::mat4 projection;
            glm::mat4 view;
            glm::mat4 lightSpaceMat;
        };

        struct UboLighting
        {
            glm::vec4 lightPos;
            glm::vec4 lightColor;
            glm::vec4 viewPos;
            float outdoorBrightness;
        };

        struct UboUI
        {
            glm::mat4 orthographicProj;
        };

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

        struct UIPushData
        {
            glm::mat4 model;
            uint32_t textureIndex;
        };

        static constexpr uint32_t POST_EFFECT_FXAA_BIT = 1u << 0;
        static constexpr uint32_t POST_EFFECT_DITHERING_BIT = 1u << 1;
        struct PostPushData
        {
            float vignetteStrength, vignetteRadius;
            uint32_t flags;
        };

        // Context
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

        VkPipelineCache pipelineCache = VK_NULL_HANDLE;

        // Pre-post images
        std::vector<ImageAttachment> prePostAttachments;

        // SwapChain
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;VkExtent2D swapChainExtent;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        uint32_t swapChainImageCount;
        VkFormat swapChainImageFormat;
        VkFormat depthFormat;

        // Runtime
        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> commandBuffers;

        uint32_t currentFrame = 0;
        bool framebufferResized = false;

        std::vector<ModelBuffer> modelBuffers;
        std::vector<WidgetBuffer> widgetBuffers;

        // Pipeline 1: Shadow
        GraphicsPipeline shadowPipeline;

        ImageAttachment shadowDepthAttachment;
        VkSampler shadowSampler = VK_NULL_HANDLE;

        // Pipeline 2: Main
        GraphicsPipeline modelPipeline;

        ImageAttachment depthAttachment;

        std::array<VkBuffer, FRAMES_IN_FLIGHT> cameraUniformBuffer;
        std::array<VkDeviceMemory, FRAMES_IN_FLIGHT> cameraUniformBufferMemory;

        std::array<VkBuffer, FRAMES_IN_FLIGHT> lightingUniformBuffer;
        std::array<VkDeviceMemory, FRAMES_IN_FLIGHT> lightingUniformBufferMemory;

        // Pipeline 3: Transparent
        GraphicsPipeline transparentPipeline;

        // Pipeline 4: UI
        GraphicsPipeline uiPipeline;

        std::array<VkBuffer, FRAMES_IN_FLIGHT> uiUniformBuffers;
        std::array<VkDeviceMemory, FRAMES_IN_FLIGHT> uiUniformBuffersMemory;

        // Pipeline 5: Post-processing
        GraphicsPipeline postPipeline;

        VkSampler postSampler = VK_NULL_HANDLE;

        // Textures
        struct TextureResources
        {
            std::vector<ImageAttachment> attachments;
            VkSampler sampler = VK_NULL_HANDLE;
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            std::vector<VkDescriptorPool> descriptorPools;
            std::vector<VkDescriptorSet> descriptorSets;
        }textures;

        // Synchronization
        std::array<VkSemaphore, FRAMES_IN_FLIGHT> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::array<VkFence, FRAMES_IN_FLIGHT> drawFences;

        std::mutex graphicsQueueMutex;
        mutable std::mutex transferQueueMutex;
        std::recursive_mutex modelMutex;
        std::recursive_mutex widgetMutex;
        std::mutex textureMutex;

        // Init
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();

        void createCommandPool();
        void createCommandBuffers();

        void createSwapChain(Size2 windowSize);

        void findDepthFormat();
        void createShadowSampler();
        void createTextureSampler();

        void createDepthAttachment();
        void createShadowDepthAttachment();

        void createTextureDescriptorSetLayout();

        void createPipelineCache();

        void createModelDescriptorSetLayout();
        void createModelPipeline();
        void createTransparentPipeline();
        void createShadowPipeline();

        void createModelUniformBuffers();
        void createModelDescriptorPool();
        void createModelDescriptorSets();

        void createPostSampler();
        void createPostDescriptorPool();
        void createPostDescriptorSetLayout();
        void createPostDescriptorSets();
        void createPostPipeline();

        void createPrePostImages();
        void createUIDescriptorSetLayout();
        void createUIPipeline();
        void createUIUniformBuffers();
        void createUIDescriptorPool();
        void createUIDescriptorSets();

        void createSemaphores();

        // Runtime
        void recordShadowPass(const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, const glm::mat4 &lightSpaceMat);
        void updateModelUniformBuffers(uint32_t currentFrame, glm::mat4 projectionMat, glm::mat4 viewMat, glm::vec4 lightPos, glm::vec3 lightColor, glm::mat4 lightSpaceMat, float outdoorBrightness);
        void recordMainPass(uint32_t currentImage, const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, color_t backgroundColor, const glm::mat4 &lightSpaceMat, const glm::vec3 &cameraPosition);
        void recordPostPass(uint32_t currentImage, const PostEffects& postEffects);
        void updateUIUniformBuffers(uint32_t currentFrame);
        void recordUIPass(uint32_t currentImage, const std::vector<Widget> &widgets, const std::vector<WidgetInstance> &widgetInstances);
        void recreateSwapChain();
        void updatePostDescriptorSets();

        // Helpers
        static void vkCheck(VkResult res, ErrorCode errorCode);
        void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory) const;
        VkResult copyBuffer(VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkFence fence) const;
        static std::vector<char> readFile(const std::string &fileName);
        VkShaderModule createShaderModule(const std::vector<char> &code) const;
        static uint32_t rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
        void destroyImageAttachment(ImageAttachment &attachment) const;

        // Models
        void syncModelBuffers(const std::vector<Model> &models);
        void createVertexBuffer(MeshBuffer &meshBuffer, const std::vector<Vertex> &vertices);
        void createIndexBuffer(MeshBuffer &meshBuffer, const std::vector<uint32_t> &indices);
        void initModelBuffer(const Model &model);
        void updateModelBuffer(ModelBuffer &modelBuffer, const Model &model);
        void removeOrphanedModel(const std::vector<ModelInstance> &modelInstances);
        void destroyMeshBuffer(MeshBuffer &meshBuffer) const;

        // UI
        void syncWidgetBuffers(const std::vector<Widget> &widgets);
        void initWidgetBuffer(const Widget &widget);

        // Textures
        void createFallbackTexture();
        size_t createTextureImage(std::string fileName);
        size_t createTexture(std::string fileName);
        size_t createTextureDescriptor(VkImageView textureImageView);
        VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory *imageMemory);
    };

}