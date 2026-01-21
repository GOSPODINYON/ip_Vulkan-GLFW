#pragma once
#include<GLFW/glfw3.h>
#include"../renderer/VulkanRenderer.h"

class Engine{
public:
    void run();
private:
    GLFWwindow* window=nullptr;
    VulkanRenderer renderer;
    void initWindow();
    void mainLoop();
    void cleanup();
};