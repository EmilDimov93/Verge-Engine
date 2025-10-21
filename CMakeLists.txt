cmake_minimum_required(VERSION 3.25)
project(VergeEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Include GLFW headers
include_directories("C:/Program Files/glfw-3.4.bin.WIN64/include")

# Add the executable
add_executable(VergeEngine 
    VergeEngine.cpp
    InputHandler.cpp
)

# Link GLFW library
target_link_libraries(VergeEngine "C:/Program Files/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib")

# Vulkan
include_directories("$ENV{VULKAN_SDK}/Include")
target_link_libraries(VergeEngine "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")