#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>

#include "LogManager.hpp"
#include "definitions.hpp"

class VulkanManager
{
public:
    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass renderPass;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    LogManager *log;

    void InitVulkan(GLFWwindow *window, Size windowSize, LogManager *logRef);

    void drawFrame();

    void cleanup();

private:
    void createInstance();
    void createSurface(GLFWwindow *window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(Size windowSize);
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSemaphores();
};