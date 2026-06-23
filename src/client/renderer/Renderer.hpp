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
#include <span>

namespace VE
{
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
        uint32_t mipLevelCount = 1;
    };

    class CommandPoolGuard
    {
    public:
        CommandPoolGuard(VkDevice device, uint32_t queueFamilyIndex) : device(device)
        {
            VkCommandPoolCreateInfo poolCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = queueFamilyIndex};
            if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
                Log::add('R', 208);
        }

        ~CommandPoolGuard()
        {
            vkDestroyCommandPool(device, pool, nullptr);
        }

        [[nodiscard]] operator VkCommandPool() const { return pool; }

    private:
        VkCommandPool pool = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
    };

    class FenceGuard
    {
    public:
        FenceGuard(VkDevice device) : device(device)
        {
            VkFenceCreateInfo fenceCreateInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            if (vkCreateFence(device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
                Log::add('R', 216);
        }

        ~FenceGuard()
        {
            vkDestroyFence(device, fence, nullptr);
        }

        [[nodiscard]] operator VkFence() const { return fence; }

        [[nodiscard]] const VkFence *ptr() const { return &fence; }

    private:
        VkFence fence = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
    };

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
        Renderer(GLFWwindow *window, Size2 windowSize);

        void drawFrame(const SceneDrawData &sceneDrawData, const UIDrawData &uiDrawData, const glm::mat4 projectionMat, const PostEffects &postEffects);

        ~Renderer();

        static void vkCheck(VkResult res, ErrorCode errorCode);

    private:
        static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

        static constexpr Size2 SHADOW_MAP_EXTENT = {4096, 4096};

        static constexpr uint32_t TEXTURE_SAMPLER_POOL_CHUNK_SIZE = 1'000;

        static constexpr char PIPELINE_CACHE_FILE_NAME[] = "pipeline_cache.bin";

        struct MeshBuffer
        {
            uint32_t vertexCount = 0;
            VkBuffer vertexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

            uint32_t indexCount = 0;
            VkBuffer indexBuffer = VK_NULL_HANDLE;
            VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

            uint32_t materialIndex = 0;

            // Used for Models, ignored for Widgets
            bool isTransparent = false;
        };

        struct ModelBuffer
        {
            ModelHandle handle;

            std::vector<MeshBuffer> meshBuffers;
            std::vector<Material> materials;

            uint64_t version = 0;

            ModelBuffer(ModelHandle handle) : handle(handle) {}
        };

        struct WidgetBuffer
        {
            WidgetHandle handle;

            std::vector<MeshBuffer> meshBuffers;
            std::vector<Material> materials;

            uint64_t version = 0;

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

        struct VertexPushData
        {
            glm::mat4 model;
            float lightStrength;
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
            uint32_t flags;
        };

        struct FrameData
        {
            VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
            VkFence drawFence = VK_NULL_HANDLE;

            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
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

        struct SwapChain
        {
            VkSwapchainKHR swapChain = VK_NULL_HANDLE;
            std::vector<VkImage> images;
            std::vector<VkImageView> imageViews;
            VkExtent2D extent;
            uint32_t imageCount;
            VkFormat imageFormat = VK_FORMAT_UNDEFINED;
            VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

            operator VkSwapchainKHR() const { return swapChain; }
        } swapChain;

        // Runtime
        VkCommandPool commandPool = VK_NULL_HANDLE;

        std::array<FrameData, FRAMES_IN_FLIGHT> frames;

        uint32_t currentFrame = 0;

        std::vector<ModelBuffer> modelBuffers;
        std::vector<WidgetBuffer> widgetBuffers;

        // Pipeline 1: Shadow
        GraphicsPipeline shadowPipeline;

        // Shadow attachments
        ImageAttachment shadowDepthAttachment;
        VkSampler shadowSampler = VK_NULL_HANDLE;

        // Pipeline 2: Model
        GraphicsPipeline modelPipeline;

        std::array<VkBuffer, FRAMES_IN_FLIGHT> cameraUniformBuffer;
        std::array<VkDeviceMemory, FRAMES_IN_FLIGHT> cameraUniformBufferMemory;

        std::array<VkBuffer, FRAMES_IN_FLIGHT> lightingUniformBuffer;
        std::array<VkDeviceMemory, FRAMES_IN_FLIGHT> lightingUniformBufferMemory;

        // Model pass attachments
        std::vector<ImageAttachment> prePostAttachments;
        std::array<ImageAttachment, FRAMES_IN_FLIGHT> depthAttachments;

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
        } textures;

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

        void createSwapChain(Size2 windowSize);

        void createShadowSampler();
        void createTextureSampler();

        void createDepthAttachments();
        void createShadowDepthAttachment();

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
        void recordShadowPass(const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, const glm::mat4 &lightSpaceMat);
        void updateModelUniformBuffers(uint32_t currentFrame, glm::mat4 projectionMat, glm::mat4 viewMat, glm::vec4 lightPos, glm::vec3 lightColor, glm::mat4 lightSpaceMat, float outdoorBrightness);
        void recordModelPass(uint32_t currentImage, const std::vector<Model> &models, const std::vector<ModelInstance> &modelInstances, color_t backgroundColor, const glm::mat4 &lightSpaceMat, const glm::vec3 &cameraPosition);
        void recordPostPass(uint32_t currentImage, const PostEffects &postEffects);
        void updateUIUniformBuffers(uint32_t currentFrame);
        void recordUIPass(uint32_t currentImage, const std::vector<Widget> &widgets, const std::vector<WidgetInstance> &widgetInstances);
        void recreateSwapChain();
        void updatePostDescriptorSets();

        // Helpers
        void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory) const;
        [[nodiscard]] VkResult copyBuffer(VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize, VkFence fence) const;
        [[nodiscard]] static uint32_t rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
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

        // UI
        void syncWidgetBuffers(const std::vector<Widget> &widgets);
        void initWidgetBuffer(const Widget &widget);
        void updateWidgetBuffer(WidgetBuffer &widgetBuffer, const Widget &widget);

        // Textures
        void createFallbackTexture();
        void createTextureDescriptorSetLayout();
        [[nodiscard]] size_t createTextureImage(const uint8_t *imageData, Size2 size);
        [[nodiscard]] size_t createTexture(std::vector<uint8_t> texturePixels, Size2 textureSize = 0);
        [[nodiscard]] size_t createTextureDescriptor(VkImageView textureImageView);
        [[nodiscard]] VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, uint32_t mipLevelCount, VkDeviceMemory *imageMemory);
    };

}