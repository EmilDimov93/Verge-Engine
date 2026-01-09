cmake_minimum_required(VERSION 3.25)
project(VergeEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(VergeEngine 
    src/Main.cpp
    
    src/Log.cpp
    src/system/Input.cpp

    src/rendering/Renderer.cpp
    src/system/WindowManager.cpp
    src/rendering/VulkanManager.cpp
    src/system/FpsManager.cpp

    src/scene/Scene.cpp
    src/actors/vehicle/VehicleCore.cpp
    src/actors/vehicle/VehicleGraphics.cpp
    src/actors/vehicle/VehiclePhysics.cpp
    src/actors/prop/Prop.cpp
    src/actors/trigger/Trigger.cpp
    src/scene/camera/Camera.cpp
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