cmake_minimum_required(VERSION 3.20)
project(VulkanWindow)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(VulkanWindow main.cpp)

target_link_libraries(VulkanWindow Vulkan::Vulkan glfw)
