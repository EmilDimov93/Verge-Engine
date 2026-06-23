// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <map>
#include <string>

namespace VE
{

inline static const std::map<std::pair<char, uint16_t>, std::string> LOG_MESSAGES{
    // Engine
    {{'E', 000}, "All resources loaded! Welcome!"},
    {{'E', 001}, "Verge Engine exited successfully"},

    {{'E', 100}, "Feature not currently supported"},
    {{'E', 101}, "Unsupported file type"},
    {{'E', 102}, "File not found"},

    {{'E', 200}, "Verge Engine crashed"},

    // Renderer
    {{'R', 000}, "Vulkan loaded successfully"},

    {{'R', 100}, "Vulkan returned: VK_NOT_READY"},
    {{'R', 101}, "Vulkan returned: VK_TIMEOUT"},
    {{'R', 102}, "Vulkan returned: VK_SUBOPTIMAL_KHR"},
    {{'R', 103}, "Vulkan returned: VK_EVENT_SET"},
    {{'R', 104}, "Vulkan returned: VK_EVENT_RESET"},
    {{'R', 110}, "Failed to write to pipeline cache file"},
    {{'R', 111}, "STB Image failed to load image"},

    {{'R', 200}, "Vulkan failed to create instance"},
    {{'R', 201}, "Vulkan failed to create window surface"},
    {{'R', 202}, "Vulkan failed to find GPU"},
    {{'R', 203}, "Vulkan failed to create logical device"},
    {{'R', 204}, "Vulkan failed to create swapchain"},
    {{'R', 205}, "Vulkan failed to create image view"},
    {{'R', 206}, "Vulkan failed to create render pass"},
    {{'R', 207}, "Vulkan failed to create framebuffer"},
    {{'R', 208}, "Vulkan failed to create command pool"},
    {{'R', 209}, "Vulkan failed to create shader"},
    {{'R', 210}, "Vulkan failed to create pipeline layout"},
    {{'R', 211}, "Vulkan failed to create graphics pipeline"},
    {{'R', 212}, "Vulkan failed to allocate command buffers"},
    {{'R', 213}, "Vulkan failed to record command buffer"},
    {{'R', 214}, "Vulkan failed to get physical device surface details"},
    {{'R', 215}, "Vulkan failed to create semaphore"},
    {{'R', 216}, "Vulkan failed to create fence"},
    {{'R', 217}, "Vulkan failed to create descriptor set layout"},
    {{'R', 218}, "Vulkan failed to create buffer"},
    {{'R', 219}, "Vulkan failed to create descriptor pool"},
    {{'R', 220}, "Vulkan failed to allocate descriptor sets"},
    {{'R', 221}, "Vulkan failed to find shader files"},
    {{'R', 222}, "Vulkan failed to create image"},
    {{'R', 223}, "Vulkan failed to find supported image format"},
    {{'R', 224}, "Vulkan failed to copy buffer"},
    {{'R', 225}, "Preferred color format unavailable"},
    {{'R', 226}, "Vulkan failed to create sampler"},
    {{'R', 227}, "Vulkan failed to create pipeline cache"},
    // {'R', 228} removed
    {{'R', 229}, "Vulkan failed to initialize mesh"},
    {{'R', 230}, "Vulkan failed to acquire swapchain image"},
    {{'R', 231}, "Vulkan failed to wait for fence"},
    {{'R', 232}, "Vulkan failed to reset fence"},
    {{'R', 233}, "Vulkan failed to submit queue"},
    {{'R', 234}, "Vulkan failed to present image"},
    {{'R', 235}, "Vulkan failed: vkDeviceWaitIdle"},
    {{'R', 236}, "Vulkan failed to map memory"},
    {{'R', 237}, "Vulkan failed to find a suitable memory type index"},
    {{'R', 238}, "Vulkan failed: vkQueueWaitIdle"},
    {{'R', 239}, "Vulkan failed to copy image"},
    {{'R', 240}, "Vulkan failed to transition image layout"},
    {{'R', 241}, "Vulkan failed to generate mipmaps"},

    // Client
    {{'C', 200}, "Renderer: invalid Field Of View value"},
    {{'C', 201}, "Renderer: invalid Z Near value"},
    {{'C', 202}, "Renderer: invalid Z Far value"},

    // GLFW
    {{'G', 000}, "GLFW loaded successfully"},

    {{'G', 100}, "Invalid key detected"},
    {{'G', 101}, "Invalid window size parameter. Using 50% of monitor size"},

    {{'G', 200}, "GLFW initialization failed"},
    {{'G', 201}, "GLFW window creation failed"},

    // Miniaudio
    {{'M', 000}, "Miniaudio loaded successfully"},
    {{'M', 100}, "Miniaudio failed to find audio file"},
    {{'M', 200}, "Miniaudio initialization failed"},

    // Scene
    {{'S', 200}, "Handle limit exceeded: too many objects"},
    {{'S', 201}, "Invalid model handle"},
    {{'S', 202}, "Invalid player handle"},
    {{'S', 203}, "Invalid vehicle handle"},
    {{'S', 204}, "Invalid prop handle"},
    {{'S', 205}, "Invalid trigger handle"},

    // Actors

    //// Vehicle
    {{'A', 100}, "Vehicle: invalid camber. Using default value"},
    {{'A', 101}, "Vehicle: invalid tire grip. Using default value"},
    {{'A', 102}, "Vehicle: invalid final drive ratio. Using default value"},
    {{'A', 103}, "Vehicle: invalid reverse gear ratio. Using default value"},
    {{'A', 104}, "Vehicle: invalid weight. Using default value"},
    {{'A', 105}, "Vehicle: invalid gear ratios. Using default value"},
    {{'A', 106}, "Vehicle: invalid braking force. Using default value"},
    {{'A', 107}, "Vehicle: invalid max RPM. Using default value"},
    {{'A', 108}, "Vehicle: invalid drivetrain efficiency. Using default value"},
    {{'A', 109}, "Vehicle: invalid wheel radius. Using default value"},
    {{'A', 110}, "Vehicle: invalid drag coefficient. Using default value"},
    {{'A', 111}, "Vehicle: invalid frontal area. Using default value"},
    {{'A', 112}, "Vehicle: invalid max steering angle. Using absolute value"},
    {{'A', 113}, "Vehicle: invalid max steering angle. Using default value"},

    //// Trigger
    {{'A', 180}, "Trigger: invalid hitbox shape"},
    {{'A', 181}, "Trigger: invalid hitbox size"},

    //// Surface
    {{'A', 190}, "Surface: invalid surface type index"},
    {{'A', 191}, "Surface: invalid friction value"},
    {{'A', 192}, "Surface: invalid color value"},

    // Widget
    {{'W', 200}, "Invalid widget handle"}};
}