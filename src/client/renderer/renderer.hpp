// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "abstractions.hpp"

#include "../../shared/drawData.hpp"

#include "../../shared/log.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <mutex>
#include <span>

namespace VE
{
    template <typename... Structs>
    void *chainStructs(Structs *...structures)
    {
        void *arr[]{structures...};
        VkBaseInStructure *last = nullptr;
        for (auto *s : arr)
        {
            auto *vk_base = static_cast<VkBaseInStructure *>(s);
            vk_base->pNext = last;
            last = vk_base;
        }
        return last;
    }

    class Renderer
    {
    public:
        Renderer(GLFWwindow *window, glm::uvec2 windowSize);

        void drawFrame(const SceneDrawData &sceneDrawData, const UIDrawData &uiDrawData, const glm::mat4 projectionMat, const PostEffects &postEffects);

        ~Renderer();

        static void vkCheck(VkResult res, ErrorCode errorCode);

    private:
        static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

        static constexpr size_t MAX_SHADOW_MAPS = 10;

        static constexpr VkExtent2D SHADOW_MAP_EXTENT = {4096, 4096};

        static constexpr uint32_t TEXTURE_SAMPLER_POOL_CHUNK_SIZE = 1'000;

        static constexpr char PIPELINE_CACHE_FILE_NAME[] = "pipeline_cache.bin";

        static constexpr VkFormat HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;

        template <typename T>
        struct PerFrame
        {
            explicit PerFrame(const uint32_t &frameIndexSource) : frameIndex(frameIndexSource) {}

            std::array<T, FRAMES_IN_FLIGHT> arr{};

            const uint32_t &frameIndex;

            operator T &() { return arr[frameIndex]; }
            operator const T &() const { return arr[frameIndex]; }

            T *operator->() { return &arr[frameIndex]; }
            const T *operator->() const { return &arr[frameIndex]; }

            T *ptr() { return &arr[frameIndex]; }
            const T *ptr() const { return &arr[frameIndex]; }
        };

        struct UboCamera
        {
            glm::mat4 projection;
            glm::mat4 view;
        };

        struct UboLighting
        {
            glm::vec4 lightPositions[MAX_SHADOW_MAPS];
            glm::vec4 lightColors[MAX_SHADOW_MAPS];
            glm::mat4 lightSpaceMats[MAX_SHADOW_MAPS];
            glm::vec4 viewPos;
            uint32_t lightCount;
            float outdoorBrightness;
        };

        struct UboUI
        {
            glm::mat4 orthographicProj;
        };

        struct VertexPushData
        {
            glm::mat4 model;
        };

        struct MaterialPushData
        {
            glm::vec4 baseColor;
            float metallic;
            float roughness;
            uint32_t textureIndex;
        };

        static constexpr uint32_t materialPushDataStructOffset = (sizeof(VertexPushData) + 15u) & ~15u;

        struct ShadowPushData
        {
            glm::mat4 model;
            glm::mat4 lightSpaceMat;
        };

        struct UIPushData
        {
            glm::mat4 model;
            uint32_t textureIndex;
            alignas(16) glm::vec4 color;
        };

        static constexpr uint32_t POST_EFFECT_FXAA_BIT = 1u << 0;
        static constexpr uint32_t POST_EFFECT_DITHERING_BIT = 1u << 1;
        struct PostPushData
        {
            float vignetteStrength, vignetteRadius;
            float exposure;
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

        VkFormat depthFormat = VK_FORMAT_UNDEFINED;

        VkPipelineCache pipelineCache = VK_NULL_HANDLE;

        SwapChain swapChain;

        // Runtime
        VkCommandPool commandPool = VK_NULL_HANDLE;

        PerFrame<VkSemaphore> imageAvailableSemaphore;
        PerFrame<VkFence> drawFence;

        PerFrame<VkCommandBuffer> commandBuffer;

        uint32_t currentFrame = 0;

        std::vector<ModelBuffer> modelBuffers;
        std::vector<WidgetBuffer> widgetBuffers;

        // Pipeline 1: Shadow
        GraphicsPipeline shadowPipeline;

        // Shadow attachments
        VkSampler shadowSampler = VK_NULL_HANDLE;

        size_t shadowMapCount = 0;
        std::array<ShadowMap, MAX_SHADOW_MAPS> shadowMaps;

        // Pipeline 2: Model
        GraphicsPipeline modelPipeline;

        PerFrame<VkBuffer> cameraUniformBuffer;
        PerFrame<VkDeviceMemory> cameraUniformBufferMemory;

        PerFrame<VkBuffer> lightingUniformBuffer;
        PerFrame<VkDeviceMemory> lightingUniformBufferMemory;

        // Model pass attachments
        std::vector<ImageAttachment> prePostAttachments;

        // Geometry Buffer
        PerFrame<GeometryBuffer> gBuffer;

        // Pipeline 3: Transparent
        GraphicsPipeline transparentPipeline;

        // Pipeline 4: UI
        GraphicsPipeline uiPipeline;

        PerFrame<VkBuffer> uiUniformBuffer;
        PerFrame<VkDeviceMemory> uiUniformBufferMemory;

        // Pipeline 5: Post-processing
        GraphicsPipeline postPipeline;

        VkSampler postSampler = VK_NULL_HANDLE;

        // Textures
        TextureResources textures;

        // Synchronization
        std::vector<VkSemaphore> renderFinishedSemaphores;

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

        void findSupportedFormats();

        void createCommandPool();
        void createCommandBuffers();

        void createSwapChain(glm::uvec2 windowSize);

        void createShadowSampler();
        void createFallbackShadowAttachment();

        void createTextureSampler();

        void createGeometryBuffers();

        void createPipelineCache();

        void createModelUniformBuffers();
        void createPostSampler();
        void createUIUniformBuffers();

        void createModelDescriptors();
        void createUIDescriptors();
        void createPostDescriptors();

        void createModelPipeline();
        void createTransparentPipeline();
        void createShadowPipeline();

        void createPostPipeline();

        void createPrePostImages();
        void createUIPipeline();

        void createSyncObjects();

        // Runtime
        void recordShadowPass(const std::vector<ModelInstance> &modelInstances, const std::array<glm::mat4, MAX_SHADOW_MAPS> &lightSpaceMats);
        void updateModelUniformBuffers(uint32_t currentFrame, const glm::mat4 &projectionMat, const glm::mat4 &viewMat, const std::array<glm::vec4, MAX_SHADOW_MAPS> &lightPositions, const std::array<glm::vec4, MAX_SHADOW_MAPS> &lightColors, const std::array<glm::mat4, MAX_SHADOW_MAPS> &lightSpaceMats, int32_t lightCount, float outdoorBrightness);
        void writeShadowDescriptors();
        void recordModelPass(uint32_t currentImage, const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, color_t backgroundColor, const glm::vec3 &cameraPosition);
        void recordPostPass(uint32_t currentImage, const PostEffects &postEffects);
        void updateUIUniformBuffers(uint32_t currentFrame);
        void recordUIPass(uint32_t currentImage, const std::vector<Widget> &widgets, const std::vector<WidgetInstance> &widgetInstances);
        void recreateSwapChain();
        void updatePostDescriptorSets();

        // Helpers
        void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory) const;
        [[nodiscard]] VkResult copyBuffer(VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkFence fence) const;
        [[nodiscard]] static uint32_t rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
        void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, std::mutex &graphicsQueueMutex, VkFence fence);
        [[nodiscard]] ImageAttachment createAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageAspectFlags aspectMask, VkExtent2D extent);
        [[nodiscard]] ShadowMap createShadowMap(ModelInstanceHandle instanceHandle);
        void destroyImageAttachment(ImageAttachment &attachment) const;
        void destroySwapChain(SwapChain swapChain) const;

        // Mesh
        [[nodiscard]] MeshBuffer createMeshBuffer(const Mesh &mesh, bool isTransparent);
        void destroyMeshBuffer(MeshBuffer &meshBuffer) const;

        // Models
        void syncModelBuffers(const std::vector<Model> &models);
        void initModelBuffer(const Model &model);
        void updateModelBuffer(ModelBuffer &modelBuffer, const Model &model);
        void removeOrphanedModel(const std::vector<ModelInstance> &modelInstances);
        bool syncShadowMaps(const std::vector<ModelInstance> &instances);

        // UI
        void syncWidgetBuffers(const std::vector<Widget> &widgets);
        void initWidgetBuffer(const Widget &widget);
        void updateWidgetBuffer(WidgetBuffer &widgetBuffer, const Widget &widget);

        // Textures
        void createFallbackTexture();
        void createTextureDescriptorSetLayout();
        [[nodiscard]] uint32_t createTexture(std::vector<uint8_t> texturePixels, glm::uvec2 textureSize);
        [[nodiscard]] uint32_t createTextureImage(const uint8_t *imageData, glm::uvec2 size);
        [[nodiscard]] uint32_t createTextureDescriptor(VkImageView textureImageView);
        [[nodiscard]] VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, uint32_t mipLevelCount, VkDeviceMemory *imageMemory);
    };
}