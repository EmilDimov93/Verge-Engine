// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <map>
#include <string>
#include <vector>

#include "definitions.hpp"

class ErrorCode
{
public:
    char letter;
    uint16_t number;

    std::string getMessage();

    ErrorCode(char newLetter, uint16_t newNumber){
        letter = newLetter;
        number = newNumber;
    }

private:
    inline static const std::map<std::pair<char, uint16_t>, std::string> messages{
        {{'C', 000}, "All resources loaded! Welcome!"},
        {{'C', 001}, "Verge Engine exited successfully"},
        {{'C', 200}, "Verge Engine crashed"},

        {{'V', 000}, "Vulkan loaded successfully"},
        {{'V', 200}, "Vulkan failed to create instance"},
        {{'V', 201}, "Vulkan failed to create window surface"},
        {{'V', 202}, "Vulkan failed to find GPU"},
        {{'V', 203}, "Vulkan failed to create logical device"},
        {{'V', 204}, "Vulkan failed to create swapchain"},
        {{'V', 205}, "Vulkan failed to create image views"},
        {{'V', 206}, "Vulkan failed to create render pass"},
        {{'V', 207}, "Vulkan failed to create framebuffer"},
        {{'V', 208}, "Vulkan failed to create command pool"},
        {{'V', 209}, "Vulkan failed to allocate command buffers"},
        {{'V', 210}, "Vulkan failed to record command buffer"},
        {{'V', 211}, "Vulkan failed to draw frame"},
        {{'V', 212}, "Vulkan failed to create shader"},
        {{'V', 213}, "Vulkan failed to get physical device surface details"},
        {{'V', 214}, "Vulkan failed to create pipeline layout"},
        {{'V', 215}, "Vulkan failed to create graphics pipeline"},

        {{'G', 000}, "GLFW loaded successfully"},
        {{'G', 200}, "GLFW initialization failed"},
        {{'G', 201}, "GLFW window creation failed"},

        {{'O', 000}, "DEV MODE ON"},
        {{'O', 100}, "Test error code"}};
};

class LogManager
{
private:
    std::vector<ErrorCode> entries;
    size_t newMessageCount = 0;
    bool hasNewMessagesFlag = false;
    size_t clearedEntriesCount = 0;

    void freeLogSpace();

public:
    void add(char letter, uint16_t number);
    std::vector<std::string> getNewMessages();
    bool hasNewMessages();
    void writeToLogFile();
    void InduceCrash();
};