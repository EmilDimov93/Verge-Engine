#pragma once

#include <map>
#include <string>
#include <vector>

class ErrorCode
{
public:
    char letter;
    int number;

    std::string GetMessage();

    ErrorCode(char l, int n) : letter(l), number(n) {}

private:
    inline static const std::map<std::pair<char, int>, std::string> messages{
        {{'O', 200}, "Invalid error code"},
        {{'O', 300}, "Test error code"},

        {{'V', 000}, "Vulkan loaded successfully"},

        {{'G', 000}, "GLFW loaded successfully"},
        {{'G', 200}, "GLFW initialization failed"},
        {{'G', 201}, "GLFW window creation failed"}};
};

class LogManager
{
private:
    std::vector<ErrorCode> logList;
    int newMessageCount = 0;
    bool hasNewMessages;

    void FreeLogSpace();

public:
    int clearedLogMessages = 0;
    void AddToLog(char letter, int number);
    std::vector<std::string> GetNewMessages();
    bool HasNewMessages();
};