#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include<iostream>
#include<vector>
#include<stdexcept>
#include<thread>
#include<atomic>
#include<cmath>

class App{
public:
    void run(){
        initWindow();
        initVulkan();

        startAsyncLoop();
        mainLoop();

        cleanup();
    }

private:
    GLFWwindow*window=nullptr;

    VkInstance instance{};
    VkSurfaceKHR surface{};
    VkPhysicalDevice physicalDevice{};
    VkDevice device{};
    VkQueue graphicsQueue{};
    uint32_t graphicsQueueFamily{};

    VkSwapchainKHR swapChain{};
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};

    VkRenderPass renderPass{};
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer>commandBuffers;

    VkSemaphore imageAvailableSemaphore{};
    VkSemaphore renderFinishedSemaphore{};

    std::vector<VkImage>swapChainImages;
    std::vector<VkImageView>swapChainImageViews;
    std::vector<VkFramebuffer>swapChainFramebuffers;

    VkClearValue clearColor{{0.0f,0.0f,0.0f,1.0f}};

    std::atomic<bool>running{true};
    std::thread asyncThread;

    std::atomic<float>r{0.0f},g{0.0f},b{0.0f};

    void startAsyncLoop(){
        asyncThread=std::thread([this]{
            while(running.load()){
                float t=static_cast<float>(glfwGetTime());
                r.store((sin(t)+1.0f)*0.5f);
                g.store((cos(t)+1.0f)*0.5f);
                b.store((sin(t*0.5f)+1.0f)*0.5f);
            }
        });
    }

    void mainLoop(){
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();

            clearColor.color={r.load(),g.load(),b.load(),1.0f};

            drawFrame();
        }

        running.store(false);
    }

    void drawFrame(){
        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            device,
            swapChain,
            UINT64_MAX,
            imageAvailableSemaphore,
            VK_NULL_HANDLE,
            &imageIndex
        );

        recordCommandBuffer(commandBuffers[imageIndex],imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[]={imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[]={
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        submitInfo.waitSemaphoreCount=1;
        submitInfo.pWaitSemaphores=waitSemaphores;
        submitInfo.pWaitDstStageMask=waitStages;
        submitInfo.commandBufferCount=1;
        submitInfo.pCommandBuffers=&commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[]={renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount=1;
        submitInfo.pSignalSemaphores=signalSemaphores;

        vkQueueSubmit(graphicsQueue,1,&submitInfo,VK_NULL_HANDLE);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount=1;
        presentInfo.pWaitSemaphores=signalSemaphores;
        presentInfo.swapchainCount=1;
        presentInfo.pSwapchains=&swapChain;
        presentInfo.pImageIndices=&imageIndex;

        vkQueuePresentKHR(graphicsQueue,&presentInfo);
        vkQueueWaitIdle(graphicsQueue);
    }

    void recordCommandBuffer(VkCommandBuffer cmd,uint32_t imageIndex){
        vkResetCommandBuffer(cmd,0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd,&beginInfo);

        VkRenderPassBeginInfo rp{};
        rp.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp.renderPass=renderPass;
        rp.framebuffer=swapChainFramebuffers[imageIndex];
        rp.renderArea.extent=swapChainExtent;
        rp.clearValueCount=1;
        rp.pClearValues=&clearColor;

        vkCmdBeginRenderPass(cmd,&rp,VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(cmd);

        vkEndCommandBuffer(cmd);
    }

    void initWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
        window=glfwCreateWindow(800,600,"Vulkan",nullptr,nullptr);
    }

    void initVulkan(){
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createRenderPass();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
    }
    
    //================================================================
    //------------------------ Vulkan helpers ------------------------
    //================================================================

    void createInstance(){
        VkApplicationInfo app{};
        app.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.apiVersion=VK_API_VERSION_1_1;

        uint32_t count;
        const char**ext=glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char*>extensions(ext,ext+count);

#ifdef __APPLE__
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

        VkInstanceCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo=&app;
        info.enabledExtensionCount=(uint32_t)extensions.size();
        info.ppEnabledExtensionNames=extensions.data();

#ifdef __APPLE__
        info.flags=VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        vkCreateInstance(&info,nullptr,&instance);
    }

    void createSurface(){
        glfwCreateWindowSurface(instance,window,nullptr,&surface);
    }

    void pickPhysicalDevice(){
        uint32_t count;
        vkEnumeratePhysicalDevices(instance,&count,nullptr);
        std::vector<VkPhysicalDevice>devs(count);
        vkEnumeratePhysicalDevices(instance,&count,devs.data());
        physicalDevice=devs[0];
    }

    void createLogicalDevice(){
        float prio=1.0f;

        VkDeviceQueueCreateInfo q{};
        q.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        q.queueFamilyIndex=0;
        q.queueCount=1;
        q.pQueuePriorities=&prio;

        const char*exts[]={VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.queueCreateInfoCount=1;
        info.pQueueCreateInfos=&q;
        info.enabledExtensionCount=1;
        info.ppEnabledExtensionNames=exts;

        vkCreateDevice(physicalDevice,&info,nullptr,&device);
        vkGetDeviceQueue(device,0,0,&graphicsQueue);
        graphicsQueueFamily=0;
    }

    void createSwapChain(){
        swapChainExtent={800,600};
        swapChainImageFormat=VK_FORMAT_B8G8R8A8_SRGB;

        VkSwapchainCreateInfoKHR info{};
        info.sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface=surface;
        info.minImageCount=2;
        info.imageFormat=swapChainImageFormat;
        info.imageColorSpace=VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        info.imageExtent=swapChainExtent;
        info.imageArrayLayers=1;
        info.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.presentMode=VK_PRESENT_MODE_FIFO_KHR;
        info.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;

        vkCreateSwapchainKHR(device,&info,nullptr,&swapChain);

        uint32_t count;
        vkGetSwapchainImagesKHR(device,swapChain,&count,nullptr);
        swapChainImages.resize(count);
        vkGetSwapchainImagesKHR(device,swapChain,&count,swapChainImages.data());
    }

    //=============================================
    //------------------ CLEANUP ------------------
    //=============================================
    
    void cleanup(){
        running.store(false);
        if(asyncThread.joinable())asyncThread.join();

        vkDeviceWaitIdle(device);

        vkDestroySemaphore(device,renderFinishedSemaphore,nullptr);
        vkDestroySemaphore(device,imageAvailableSemaphore,nullptr);

        for(auto fb:swapChainFramebuffers)
            vkDestroyFramebuffer(device,fb,nullptr);
        for(auto iv:swapChainImageViews)
            vkDestroyImageView(device,iv,nullptr);

        vkDestroyCommandPool(device,commandPool,nullptr);
        vkDestroyRenderPass(device,renderPass,nullptr);
        vkDestroySwapchainKHR(device,swapChain,nullptr);
        vkDestroyDevice(device,nullptr);
        vkDestroySurfaceKHR(instance,surface,nullptr);
        vkDestroyInstance(instance,nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main(){
    App app;
    app.run();
    return 0;
}
