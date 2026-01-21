cmake_minimum_required(VERSION 3.20)

project(VulkanEngine
    VERSION 1.0
    DESCRIPTION "Vulkan + GLFW C++ Game Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------
#  Dependencies
# -----------------------------
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# -----------------------------
#  Source files
# -----------------------------
set(SOURCES
    src/app/main.cpp
    src/engine/Engine.cpp
    src/renderer/VulkanRenderer.cpp
)

# -----------------------------
#  Target
# -----------------------------
add_executable(VulkanEngine
    ${SOURCES}
)

# -----------------------------
#  Include directories
# -----------------------------
target_include_directories(VulkanEngine
    PRIVATE
        src
)

# -----------------------------
#  Link libraries
# -----------------------------
target_link_libraries(VulkanEngine
    PRIVATE
        Vulkan::Vulkan
        glfw
)

# -----------------------------
#  Build output
# -----------------------------
set_target_properties(VulkanEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)