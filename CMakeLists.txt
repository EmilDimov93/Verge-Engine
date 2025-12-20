cmake_minimum_required(VERSION 3.25)
project(VergeEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(VergeEngine 
    src/core/Core.cpp
    src/core/Log.cpp

    src/system/WindowManager.cpp
    src/system/Input.cpp
    src/system/FpsManager.cpp
    
    src/rendering/VulkanManager.cpp
    src/rendering/Camera.cpp
    src/rendering/Mesh.cpp

    src/vehicle/VehicleCore.cpp
    src/vehicle/VehicleGraphics.cpp
    src/vehicle/VehiclePhysics.cpp

    src/scene/Scene.cpp
)

target_include_directories(VergeEngine PRIVATE
    "C:/Program Files/glfw-3.4.bin.WIN64/include"
    "$ENV{VULKAN_SDK}/Include"
    ext/glm
)

target_link_libraries(VergeEngine PRIVATE
    "C:/Program Files/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib"
    "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib"
)