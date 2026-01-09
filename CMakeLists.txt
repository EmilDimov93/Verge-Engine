cmake_minimum_required(VERSION 3.25)
project(VergeEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(VergeEngine 
    src/Main.cpp
    
    src/shared/Log.cpp
    src/shared/Input.cpp

    src/renderer/Renderer.cpp
    src/renderer/WindowManager.cpp
    src/renderer/VulkanManager.cpp
    src/renderer/FpsManager.cpp

    src/scene/Scene.cpp
    src/scene/actors/VehicleCore.cpp
    src/scene/actors/VehicleGraphics.cpp
    src/scene/actors/VehiclePhysics.cpp
    src/scene/actors/Prop.cpp
    src/scene/actors/Trigger.cpp
    src/scene/Camera.cpp
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