cmake_minimum_required(VERSION 3.20)
project(VulkanWindow LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vulkan
find_package(Vulkan REQUIRED)

# GLFW
find_package(glfw3 CONFIG REQUIRED)

add_executable(VulkanWindow
    main.cpp
)

target_link_libraries(VulkanWindow
    Vulkan::Vulkan
    glfw
)

# macOS specific
if(APPLE)
    target_link_libraries(VulkanWindow
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()
