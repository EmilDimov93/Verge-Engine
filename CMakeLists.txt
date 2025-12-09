cmake_minimum_required(VERSION 3.25)
project(VergeEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(VergeEngine 
    src/Core.cpp
    src/VulkanManager.cpp
    src/WindowManager.cpp
    src/Mesh.cpp
    src/Input.cpp
    src/FpsManager.cpp
    src/Log.cpp
)

target_include_directories(VergeEngine PRIVATE
    "C:/Program Files/glfw-3.4.bin.WIN64/include"
    "$ENV{VULKAN_SDK}/Include"
    externals/glm
)

target_link_libraries(VergeEngine PRIVATE
    "C:/Program Files/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib"
    "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib"
)