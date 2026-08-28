cmake_minimum_required(VERSION 3.25)
project(VergeEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(VergeEngine 
    src/Example.cpp
    
    src/shared/log.cpp

    src/client/client.cpp
    src/client/window.cpp
    src/client/audioManager.cpp
    src/client/input.cpp
    src/client/fpsManager.cpp
    src/client/ui.cpp
    src/client/renderer/init.cpp
    src/client/renderer/initDescriptors.cpp
    src/client/renderer/initPipelines.cpp
    src/client/renderer/runtime.cpp
    src/client/renderer/helpers.cpp
    src/client/renderer/mesh.cpp
    src/client/renderer/models.cpp
    src/client/renderer/widgets.cpp
    src/client/renderer/textures.cpp

    src/scene/sceneCore.cpp
    src/scene/sceneActors.cpp
    src/scene/player.cpp
    src/scene/actors/vehicleCore.cpp
    src/scene/actors/vehiclePhysics.cpp
    src/scene/actors/prop.cpp
    src/scene/actors/trigger.cpp

    src/shared/modelLoader.cpp
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