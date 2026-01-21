#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<vulkan/vulkan.h>
#include<vector>
#include<thread>
#include<atomic>
#include<cstdint>

class VulkanRenderer{
public:
    void init(GLFWwindow* window);
    void drawFrame();
    void cleanup();

private:
    GLFWwindow* window=nullptr;

    VkInstance instance{};
    VkSurfaceKHR surface{};
    VkPhysicalDevice physicalDevice{};
    VkDevice device{};
    VkQueue graphicsQueue{};
    uint32_t graphicsQueueFamily=0;

    VkSwapchainKHR swapChain{};
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};

    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass{};
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers;

    VkSemaphore imageAvailableSemaphore{};
    VkSemaphore renderFinishedSemaphore{};

    VkClearValue clearColor{{0.f,0.f,1.f,1.f}};

    std::atomic<bool> running{true};
    std::thread asyncThread;
    std::atomic<float> r{0.f},g{0.f},b{0.f};

    void startAsyncLoop();
    void initVulkan();

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    void recordCommandBuffer(VkCommandBuffer cmd,uint32_t index);
};