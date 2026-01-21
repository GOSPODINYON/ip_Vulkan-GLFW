#include"Engine.h"

void Engine::run(){
    initWindow();
    renderer.init(window);
    mainLoop();
    cleanup();
}

void Engine::initWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
    window=glfwCreateWindow(800,600,"Vulkan",nullptr,nullptr);
}

void Engine::mainLoop(){
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        renderer.drawFrame();
    }
}

void Engine::cleanup(){
    renderer.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}