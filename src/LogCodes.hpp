// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <map>
#include <string>

inline static const std::map<std::pair<char, uint16_t>, std::string> LOG_MESSAGES{
    // Core
    {{'C', 000}, "All resources loaded! Welcome!"},
    {{'C', 001}, "Verge Engine exited successfully"},

    {{'C', 100}, "Invalid key detected"},

    {{'C', 200}, "Verge Engine crashed"},

    // Vulkan
    {{'V', 000}, "Vulkan loaded successfully"},

    {{'V', 100}, "Vulkan returned: VK_NOT_READY"},
    {{'V', 101}, "Vulkan returned: VK_TIMEOUT"},
    {{'V', 102}, "Vulkan returned: VK_SUBOPTIMAL_KHR"},
    {{'V', 103}, "Vulkan returned: VK_EVENT_SET"},
    {{'V', 104}, "Vulkan returned: VK_EVENT_RESET"},

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
    {{'V', 229}, "Vulkan failed to initialize mesh"},
    {{'V', 230}, "Vulkan failed to acquire swapchain image"},
    {{'V', 231}, "Vulkan failed to wait for fence"},
    {{'V', 232}, "Vulkan failed to reset fence"},
    {{'V', 233}, "Vulkan failed to submit graphics queue"},
    {{'V', 234}, "Vulkan failed to present image"},
    {{'V', 235}, "Vulkan failed: vkDeviceWaitIdle"},
    {{'V', 236}, "Vulkan failed to map memory"},

    // GLFW
    {{'G', 000}, "GLFW loaded successfully"},

    {{'G', 200}, "GLFW initialization failed"},
    {{'G', 201}, "GLFW window creation failed"},

    // Other
    {{'O', 100}, "Invalid error code"}};