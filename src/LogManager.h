#pragma once

#include <map>
#include <string>
#include <vector>

class ErrorCode
{
public:
    char letter;
    int number;

    std::string getMessage();

    ErrorCode(char l, int n) : letter(l), number(n) {}

private:
    inline static const std::map<std::pair<char, int>, std::string> messages{
        {{'C', 000}, "Verge Engine loaded successfully"},

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

        {{'G', 000}, "GLFW loaded successfully"},
        {{'G', 200}, "GLFW initialization failed"},
        {{'G', 201}, "GLFW window creation failed"},

        {{'O', 200}, "Invalid error code"},
        {{'O', 300}, "Test error code"}};
};

class LogManager
{
private:
    std::vector<ErrorCode> entries;
    int newMessageCount = 0;
    bool hasNewMessagesFlag = false;
    int clearedEntriesCount = 0;

    void freeLogSpace();

public:
    void add(char letter, int number);
    std::vector<std::string> getNewMessages();
    bool hasNewMessages();
    void writeToLogFile();
};