#pragma once

#include <map>

class ErrorCode
{
public:
    char letter;
    int number;

    std::string GetMessage(){
        return messages.find({letter, number})->second;
    }

    ErrorCode(char l, int n) : letter(l), number(n) {}

private:
    inline static const std::map<std::pair<char,int>, std::string> messages{//Vulkan:
        {{'V', 000}, "Vulkan loaded successfully"},

        //GLFW:
        {{'G', 000}, "GLFW loaded successfully"},
        {{'G', 200}, "GLFW initialization failed"},
        {{'G', 201}, "GLFW window creation failed"}
    };
};