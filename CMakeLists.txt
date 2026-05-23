cmake_minimum_required(VERSION 3.25)
project(VergeEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(VergeEngine 
    src/Example.cpp
    
    src/shared/Log.cpp

    src/client/Client.cpp
    src/client/WindowManager.cpp
    src/client/AudioManager.cpp
    src/client/Input.cpp
    src/client/FpsManager.cpp
    src/client/renderer/RendererInit.cpp
    src/client/renderer/RendererInitPipelines.cpp
    src/client/renderer/RendererRuntime.cpp
    src/client/renderer/RendererHelpers.cpp
    src/client/renderer/RendererModels.cpp
    src/client/renderer/RendererUI.cpp
    src/client/renderer/RendererTextures.cpp

    src/scene/SceneCore.cpp
    src/scene/SceneActors.cpp
    src/scene/Player.cpp
    src/scene/Camera.cpp
    src/scene/actors/VehicleCore.cpp
    src/scene/actors/VehiclePhysics.cpp
    src/scene/actors/Prop.cpp
    src/scene/actors/Trigger.cpp

    src/ui/UI.cpp
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