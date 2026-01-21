#include"VulkanRenderer.h"
#include<cmath>
#include<vector>
#include<thread>
#include<atomic>

void VulkanRenderer::init(GLFWwindow* win){window=win;initVulkan();startAsyncLoop();}

void VulkanRenderer::drawFrame(){
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device,swapChain,UINT64_MAX,imageAvailableSemaphore,VK_NULL_HANDLE,&imageIndex);
    recordCommandBuffer(commandBuffers[imageIndex],imageIndex);

    VkPipelineStageFlags waitStage=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit{};
    submit.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount=1;
    submit.pWaitSemaphores=&imageAvailableSemaphore;
    submit.pWaitDstStageMask=&waitStage;
    submit.commandBufferCount=1;
    submit.pCommandBuffers=&commandBuffers[imageIndex];
    submit.signalSemaphoreCount=1;
    submit.pSignalSemaphores=&renderFinishedSemaphore;

    vkQueueSubmit(graphicsQueue,1,&submit,VK_NULL_HANDLE);

    VkPresentInfoKHR present{};
    present.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount=1;
    present.pWaitSemaphores=&renderFinishedSemaphore;
    present.swapchainCount=1;
    present.pSwapchains=&swapChain;
    present.pImageIndices=&imageIndex;

    vkQueuePresentKHR(graphicsQueue,&present);
    vkQueueWaitIdle(graphicsQueue);
}

void VulkanRenderer::cleanup(){
    running.store(false);
    if(asyncThread.joinable())asyncThread.join();
    vkDeviceWaitIdle(device);

    vkDestroySemaphore(device,renderFinishedSemaphore,nullptr);
    vkDestroySemaphore(device,imageAvailableSemaphore,nullptr);

    for(auto fb:swapChainFramebuffers)vkDestroyFramebuffer(device,fb,nullptr);
    for(auto iv:swapChainImageViews)vkDestroyImageView(device,iv,nullptr);

    vkDestroyCommandPool(device,commandPool,nullptr);
    vkDestroyRenderPass(device,renderPass,nullptr);
    vkDestroySwapchainKHR(device,swapChain,nullptr);
    vkDestroyDevice(device,nullptr);
    vkDestroySurfaceKHR(instance,surface,nullptr);
    vkDestroyInstance(instance,nullptr);
}

void VulkanRenderer::startAsyncLoop(){
    asyncThread=std::thread([this]{
        while(running.load()){
            float t=(float)glfwGetTime();
            r.store((sin(t)+1.f)*0.5f);
            g.store((cos(t)+1.f)*0.5f);
            b.store((sin(t*0.5f)+1.f)*0.5f);
        }
    });
}

void VulkanRenderer::initVulkan(){
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

void VulkanRenderer::createInstance(){
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

void VulkanRenderer::createSurface(){glfwCreateWindowSurface(instance,window,nullptr,&surface);}

void VulkanRenderer::pickPhysicalDevice(){
    uint32_t count;
    vkEnumeratePhysicalDevices(instance,&count,nullptr);
    std::vector<VkPhysicalDevice>devs(count);
    vkEnumeratePhysicalDevices(instance,&count,devs.data());
    physicalDevice=devs[0];
}

void VulkanRenderer::createLogicalDevice(){
    float prio=1.f;
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
}

void VulkanRenderer::createSwapChain(){
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
    info.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;
    info.presentMode=VK_PRESENT_MODE_FIFO_KHR;

    vkCreateSwapchainKHR(device,&info,nullptr,&swapChain);

    uint32_t count;
    vkGetSwapchainImagesKHR(device,swapChain,&count,nullptr);
    swapChainImages.resize(count);
    vkGetSwapchainImagesKHR(device,swapChain,&count,swapChainImages.data());
}

void VulkanRenderer::createRenderPass(){
    VkAttachmentDescription color{};
    color.format=swapChainImageFormat;
    color.samples=VK_SAMPLE_COUNT_1_BIT;
    color.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
    color.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ref{0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription sub{};
    sub.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount=1;
    sub.pColorAttachments=&ref;

    VkRenderPassCreateInfo info{};
    info.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount=1;
    info.pAttachments=&color;
    info.subpassCount=1;
    info.pSubpasses=&sub;

    vkCreateRenderPass(device,&info,nullptr,&renderPass);
}

void VulkanRenderer::createFramebuffers(){
    swapChainImageViews.resize(swapChainImages.size());
    swapChainFramebuffers.resize(swapChainImages.size());

    for(size_t i=0;i<swapChainImages.size();i++){
        VkImageViewCreateInfo view{};
        view.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.image=swapChainImages[i];
        view.viewType=VK_IMAGE_VIEW_TYPE_2D;
        view.format=swapChainImageFormat;
        view.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.levelCount=1;
        view.subresourceRange.layerCount=1;

        vkCreateImageView(device,&view,nullptr,&swapChainImageViews[i]);

        VkImageView att[]={swapChainImageViews[i]};

        VkFramebufferCreateInfo fb{};
        fb.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb.renderPass=renderPass;
        fb.attachmentCount=1;
        fb.pAttachments=att;
        fb.width=swapChainExtent.width;
        fb.height=swapChainExtent.height;
        fb.layers=1;

        vkCreateFramebuffer(device,&fb,nullptr,&swapChainFramebuffers[i]);
    }
}

void VulkanRenderer::createCommandPool(){
    VkCommandPoolCreateInfo info{};
    info.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex=graphicsQueueFamily;
    vkCreateCommandPool(device,&info,nullptr,&commandPool);
}

void VulkanRenderer::createCommandBuffers(){
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo info{};
    info.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool=commandPool;
    info.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount=(uint32_t)commandBuffers.size();

    vkAllocateCommandBuffers(device,&info,commandBuffers.data());
}

void VulkanRenderer::createSyncObjects(){
    VkSemaphoreCreateInfo info{};
    info.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device,&info,nullptr,&imageAvailableSemaphore);
    vkCreateSemaphore(device,&info,nullptr,&renderFinishedSemaphore);
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer cmd,uint32_t index){
    vkResetCommandBuffer(cmd,0);
    clearColor.color={r.load(),g.load(),b.load(),1.f};

    VkCommandBufferBeginInfo begin{};
    begin.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd,&begin);

    VkRenderPassBeginInfo rp{};
    rp.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp.renderPass=renderPass;
    rp.framebuffer=swapChainFramebuffers[index];
    rp.renderArea.extent=swapChainExtent;
    rp.clearValueCount=1;
    rp.pClearValues=&clearColor;

    vkCmdBeginRenderPass(cmd,&rp,VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);
}