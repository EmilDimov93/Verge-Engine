// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <map>
#include <string>

inline static const std::map<std::pair<char, uint16_t>, std::string> LOG_MESSAGES{
    // Engine
    {{'E', 000}, "All resources loaded! Welcome!"},
    {{'E', 001}, "Verge Engine exited successfully"},

    {{'E', 100}, "Temporary code"},

    {{'E', 200}, "Verge Engine crashed"},

    // Vulkan
    {{'V', 000}, "Vulkan loaded successfully"},

    {{'V', 100}, "Vulkan returned: VK_NOT_READY"},
    {{'V', 101}, "Vulkan returned: VK_TIMEOUT"},
    {{'V', 102}, "Vulkan returned: VK_SUBOPTIMAL_KHR"},
    {{'V', 103}, "Vulkan returned: VK_EVENT_SET"},
    {{'V', 104}, "Vulkan returned: VK_EVENT_RESET"},
    {{'V', 120}, "Vulkan: Mesh buffers were not destroyed at exit"},

    {{'V', 200}, "Vulkan failed to create instance"},
    {{'V', 201}, "Vulkan failed to create window surface"},
    {{'V', 202}, "Vulkan failed to find GPU"},
    {{'V', 203}, "Vulkan failed to create logical device"},
    {{'V', 204}, "Vulkan failed to create swapchain"},
    {{'V', 205}, "Vulkan failed to create image views"},
    {{'V', 206}, "Vulkan failed to create render pass"},
    {{'V', 207}, "Vulkan failed to create framebuffer"},
    {{'V', 208}, "Vulkan failed to create command pool"},
    {{'V', 209}, "Vulkan failed to create shader"},
    {{'V', 210}, "Vulkan failed to create pipeline layout"},
    {{'V', 211}, "Vulkan failed to create graphics pipeline"},
    {{'V', 212}, "Vulkan failed to allocate command buffers"},
    {{'V', 213}, "Vulkan failed to record command buffer"},
    {{'V', 214}, "Vulkan failed to get physical device surface details"},
    {{'V', 215}, "Vulkan failed to create semaphore"},
    {{'V', 216}, "Vulkan failed to create fence"},
    {{'V', 217}, "Vulkan failed to create descriptor set layout"},
    {{'V', 218}, "Vulkan failed to create buffer"}, //
    {{'V', 219}, "Vulkan failed to create descriptor pool"},
    {{'V', 220}, "Vulkan failed to allocate descriptor sets"},
    {{'V', 221}, "Vulkan failed to find shader files"},
    {{'V', 222}, "Vulkan failed to create depth buffer image"},
    {{'V', 223}, "Vulkan failed to find supported image format"},
    {{'V', 229}, "Vulkan failed to initialize mesh"},
    {{'V', 230}, "Vulkan failed to acquire swapchain image"},
    {{'V', 231}, "Vulkan failed to wait for fence"},
    {{'V', 232}, "Vulkan failed to reset fence"},
    {{'V', 233}, "Vulkan failed to submit graphics queue"},
    {{'V', 234}, "Vulkan failed to present image"},
    {{'V', 235}, "Vulkan failed: vkDeviceWaitIdle"},
    {{'V', 236}, "Vulkan failed to map memory"},
    {{'V', 237}, "Vulkan failed to find a suitable memory type index"},
    {{'V', 238}, "Vulkan failed: vkQueueWaitIdle"},

    // Camera
    {{'K', 100}, "Camera already initialized. Operation cancelled"},
    {{'K', 200}, "Camera not initialized"},
    {{'K', 201}, "Camera: invalid Field Of View value"}, // TODO: Change to error?
    {{'K', 202}, "Camera: invalid Aspect Ratio value"},  // TODO: Change to error?
    {{'K', 203}, "Camera: invalid Z Near value"},        // TODO: Change to error?
    {{'K', 204}, "Camera: invalid Z Far value"},         // TODO: Change to error?

    // GLFW
    {{'G', 000}, "GLFW loaded successfully"},

    {{'G', 100}, "Window not initialized. getAspectRatio returns 1.0f"},
    {{'G', 101}, "Invalid key detected"},
    {{'G', 102}, "Invalid window size parameter. Using 50%"},

    {{'G', 200}, "GLFW initialization failed"},
    {{'G', 201}, "GLFW window creation failed"},

    // Scene
    {{'S', 100}, "Unsupported file type"},
    {{'S', 101}, "File not found"},
    {{'S', 102}, "Out of bounds access to mesh list"},
    {{'S', 103}, "Out of bounds access to vehicle list"},
    {{'S', 200}, "Invalid mesh index"},
    {{'S', 201}, "Trigger ID already exists"},

    // Actors

    // Vehicle
    {{'A', 100}, "Vehicle: missing mesh for body"},
    {{'A', 101}, "Vehicle: missing mesh for tire"},
    {{'A', 102}, "Vehicle: invalid power unit. Using default KILOWATTS"},
    {{'A', 103}, "Vehicle: invalid power. Using default value"},
    {{'A', 104}, "Vehicle: invalid weight. Using default value"},
    {{'A', 105}, "Vehicle: invalid gear count. Using default value"},
    {{'A', 106}, "Vehicle: invalid idle RPM. Using default value"},
    {{'A', 107}, "Vehicle: invalid max RPM. Using default value"},
    {{'A', 108}, "Vehicle: invalid drivetrain efficiency. Using default value"},
    {{'A', 109}, "Vehicle: invalid wheel radius. Using default value"},
    {{'A', 110}, "Vehicle: invalid drag coefficient. Using default value"},
    {{'A', 111}, "Vehicle: invalid frontal area. Using default value"},
    {{'A', 112}, "Vehicle: invalid max steering angle. Using absolute value"},
    {{'A', 113}, "Vehicle: invalid max steering angle. Using default value"},
    {{'A', 114}, "Vehicle: invalid camber. Using default value"},
    {{'A', 115}, "Vehicle: invalid tire grip. Using default value"},

    // Prop
    {{'A', 160}, "Prop: invalid mesh index"},

    // Trigger
    {{'A', 180}, "Trigger: invalid ID"},
    {{'A', 181}, "Trigger: invalid mesh index"},
    {{'A', 182}, "Trigger: invalid hitbox shape"},
    {{'A', 183}, "Trigger: invalid hitbox size"},

    // Ground
    {{'A', 190}, "Ground: invalid surface index"},
    {{'A', 191}, "Ground: invalid coordinates for vertex"},
    {{'A', 192}, "Surface: invalid friction value"},
    {{'A', 193}, "Surface: invalid color value"},
    {{'A', 194}, "Surface: invalid color distortion value"}};