#pragma once

#include <map>
#include <string>

class ErrorCode
{
public:
    char letter;
    int number;

    std::string GetMessage()
    {
        auto it = messages.find({letter, number});
        if (it != messages.end())
            return it->second;
        return "";
    }

    ErrorCode(char l, int n) : letter(l), number(n) {}

private:
    inline static const std::map<std::pair<char, int>, std::string> messages{
        {{'O', 200}, "Invalid error code"},

        {{'V', 000}, "Vulkan loaded successfully"},

        {{'G', 000}, "GLFW loaded successfully"},
        {{'G', 200}, "GLFW initialization failed"},
        {{'G', 201}, "GLFW window creation failed"}};
};